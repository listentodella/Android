
#include "HeapCache.h"
#include "IBinder.h"
#include "IMemory.h"

namespace android {

//***************************************************************************

HeapCache::HeapCache()
    : DeathRecipient()
{
}

HeapCache::~HeapCache(){
}

void HeapCache::binderDied(const wp<IBinder> & binder) {
    //ALOGD("binderDied binder=%p", binder.unsafe_get());
    free_heap(binder);
}

sp<IMemoryHeap> HeapCache::find_heap(const sp<IBinder> & binder) {
    Mutex::Autolock _l(mHeapCacheLock);
    ssize_t i = mHeapCache.indexOfKey(binder);
    if (i>=0) {
        heap_info_t& info = mHeapCache.editValueAt(i);
        ALOGD_IF(VERBOSE,
                "found binder=%p, heap=%p, size=%zu, fd=%d, count=%d",
                binder.get(), info.heap.get(),
                static_cast<BpMemoryHeap*>(info.heap.get())->mSize,
                static_cast<BpMemoryHeap*>(info.heap.get())->mHeapId,
                info.count);
        android_atomic_inc(&info.count);
        return info.heap;
    } else {
        heap_info_t info;
        info.heap = interface_cast<IMemoryHeap>(binder);
        info.count = 1;
        //ALOGD("adding binder=%p, heap=%p, count=%d",
        //      binder.get(), info.heap.get(), info.count);
        mHeapCache.add(binder, info);
        return info.heap;
    }
}

void HeapCache::free_heap(const sp<IBinder> & binder) {
    free_heap( wp<IBinder>(binder) );
}

sp<IMemoryHeap> HeapCache::get_heap(const sp<IBinder> & binder) {
    sp<IMemoryHeap> realHeap;
    Mutex::Autolock _l(mHeapCacheLock);
    ssize_t i = mHeapCache.indexOfKey(binder);
    if (i>=0)   realHeap = mHeapCache.valueAt(i).heap;
    else        realHeap = interface_cast<IMemoryHeap>(binder);
    return realHeap;
}

void HeapCache::dump_heaps() {
    Mutex::Autolock _l(mHeapCacheLock);
    int c = mHeapCache.size();
    for (int i=0 ; i<c ; i++) {
        const heap_info_t& info = mHeapCache.valueAt(i);
        BpMemoryHeap const* h(static_cast<BpMemoryHeap const *>(info.heap.get()));
        ALOGD("hey=%p, heap=%p, count=%d, (fd=%d, base=%p, size=%zu)",
                mHeapCache.keyAt(i).unsafe_get(),
                info.heap.get(), info.count,
                h->mHeapId, h->mBase, h->mSize);
    }
}

void HeapCache::free_heap(const wp<IBinder> & binder) {
    sp<IMemoryHeap> rel;
    {
        Mutex::Autolock _l(mHeapCacheLock);
        ssize_t i = mHeapCache.indexOfKey(binder);
        if (i>=0) {
            heap_info_t& info(mHeapCache.editValueAt(i));
            int32_t c = android_atomic_dec(&info.count);
            if (c == 1) {
                ALOGD_IF(VERBOSE,
                        "removing binder=%p, heap=%p, size=%zu, fd=%d, count=%d",
                        binder.unsafe_get(), info.heap.get(),
                        static_cast<BpMemoryHeap*>(info.heap.get())->mSize,
                        static_cast<BpMemoryHeap*>(info.heap.get())->mHeapId,
                        info.count);
                rel = mHeapCache.valueAt(i).heap;
                mHeapCache.removeItemsAt(i);
            }
        } else {
            ALOGE("free_heap binder=%p not found!!!", binder.unsafe_get());
        }
    }
}


} // namespace android
