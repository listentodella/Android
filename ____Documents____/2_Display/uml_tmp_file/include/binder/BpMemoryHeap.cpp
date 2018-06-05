
#include "BpMemoryHeap.h"
#include "IBinder.h"
#include "IMemory.h"

namespace android {

//****************************************************************************

BpMemoryHeap::BpMemoryHeap(const sp<IBinder> & impl)
    : BpInterface<IMemoryHeap>(impl),
        mHeapId(-1), mBase(MAP_FAILED), mSize(0), mFlags(0), mOffset(0), mRealHeap(false)
{
}

BpMemoryHeap::~BpMemoryHeap(){
    if (mHeapId != -1) {
        close(mHeapId);
        if (mRealHeap) {
            // by construction we're the last one
            if (mBase != MAP_FAILED) {
                sp<IBinder> binder = const_cast<BpMemoryHeap*>(this)->asBinder();

                if (VERBOSE) {
                    ALOGD("UNMAPPING binder=%p, heap=%p, size=%zu, fd=%d",
                            binder.get(), this, mSize, mHeapId);
                    CallStack stack(LOG_TAG);
                }

                munmap(mBase, mSize);
            }
        } else {
            // remove from list only if it was mapped before
            sp<IBinder> binder = const_cast<BpMemoryHeap*>(this)->asBinder();
            free_heap(binder);
        }
    }
}

int BpMemoryHeap::getHeapID() const {
    assertMapped();
    return mHeapId;
}

void * BpMemoryHeap::getBase() const {
    assertMapped();
    return mBase;
}

size_t BpMemoryHeap::getSize() const {
    assertMapped();
    return mSize;
}

uint32_t BpMemoryHeap::getFlags() const {
    assertMapped();
    return mFlags;
}

uint32_t BpMemoryHeap::getOffset() const {
    assertMapped();
    return mOffset;
}

void BpMemoryHeap::assertMapped() const {
    if (mHeapId == -1) {
        sp<IBinder> binder(const_cast<BpMemoryHeap*>(this)->asBinder());
        sp<BpMemoryHeap> heap(static_cast<BpMemoryHeap*>(find_heap(binder).get()));
        heap->assertReallyMapped();
        if (heap->mBase != MAP_FAILED) {
            Mutex::Autolock _l(mLock);
            if (mHeapId == -1) {
                mBase   = heap->mBase;
                mSize   = heap->mSize;
                mOffset = heap->mOffset;
                android_atomic_write( dup( heap->mHeapId ), &mHeapId );
            }
        } else {
            // something went wrong
            free_heap(binder);
        }
    }
}

void BpMemoryHeap::assertReallyMapped() const {
    if (mHeapId == -1) {

        // remote call without mLock held, worse case scenario, we end up
        // calling transact() from multiple threads, but that's not a problem,
        // only mmap below must be in the critical section.

        Parcel data, reply;
        data.writeInterfaceToken(IMemoryHeap::getInterfaceDescriptor());
        status_t err = remote()->transact(HEAP_ID, data, &reply);
        int parcel_fd = reply.readFileDescriptor();
        ssize_t size = reply.readInt32();
        uint32_t flags = reply.readInt32();
        uint32_t offset = reply.readInt32();

        ALOGE_IF(err, "binder=%p transaction failed fd=%d, size=%zd, err=%d (%s)",
                asBinder().get(), parcel_fd, size, err, strerror(-err));

        int fd = dup( parcel_fd );
        ALOGE_IF(fd==-1, "cannot dup fd=%d, size=%zd, err=%d (%s)",
                parcel_fd, size, err, strerror(errno));

        int access = PROT_READ;
        if (!(flags & READ_ONLY)) {
            access |= PROT_WRITE;
        }

        Mutex::Autolock _l(mLock);
        if (mHeapId == -1) {
            mRealHeap = true;
            mBase = mmap(0, size, access, MAP_SHARED, fd, offset);
            if (mBase == MAP_FAILED) {
                ALOGE("cannot map BpMemoryHeap (binder=%p), size=%zd, fd=%d (%s)",
                        asBinder().get(), size, fd, strerror(errno));
                close(fd);
            } else {
                mSize = size;
                mFlags = flags;
                mOffset = offset;
                android_atomic_write(fd, &mHeapId);
            }
        }
    }
}


} // namespace android
