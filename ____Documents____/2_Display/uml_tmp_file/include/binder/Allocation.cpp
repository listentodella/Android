
#include "Allocation.h"
#include "MemoryDealer.h"
#include "IMemory.h"

namespace android {

// ----------------------------------------------------------------------------

Allocation::Allocation(const sp<MemoryDealer> & dealer, const sp<IMemoryHeap> & heap, ssize_t offset, size_t size)
    : MemoryBase(heap, offset, size), mDealer(dealer)
{
#ifndef NDEBUG
    void* const start_ptr = (void*)(intptr_t(heap->base()) + offset);
    memset(start_ptr, 0xda, size);
#endif
}

Allocation::~Allocation(){
    size_t freedOffset = getOffset();
    size_t freedSize   = getSize();
    if (freedSize) {
        /* NOTE: it's VERY important to not free allocations of size 0 because
         * they're special as they don't have any record in the allocator
         * and could alias some real allocation (their offset is zero). */

        // keep the size to unmap in excess
        size_t pagesize = getpagesize();
        size_t start = freedOffset;
        size_t end = start + freedSize;
        start &= ~(pagesize-1);
        end = (end + pagesize-1) & ~(pagesize-1);

        // give back to the kernel the pages we don't need
        size_t free_start = freedOffset;
        size_t free_end = free_start + freedSize;
        if (start < free_start)
            start = free_start;
        if (end > free_end)
            end = free_end;
        start = (start + pagesize-1) & ~(pagesize-1);
        end &= ~(pagesize-1);

        if (start < end) {
            void* const start_ptr = (void*)(intptr_t(getHeap()->base()) + start);
            size_t size = end-start;

#ifndef NDEBUG
            memset(start_ptr, 0xdf, size);
#endif

            // MADV_REMOVE is not defined on Dapper based Goobuntu
#ifdef MADV_REMOVE
            if (size) {
                int err = madvise(start_ptr, size, MADV_REMOVE);
                ALOGW_IF(err, "madvise(%p, %zu, MADV_REMOVE) returned %s",
                        start_ptr, size, err<0 ? strerror(errno) : "Ok");
            }
#endif
        }

        // This should be done after madvise(MADV_REMOVE), otherwise madvise()
        // might kick out the memory region that's allocated and/or written
        // right after the deallocation.
        mDealer->deallocate(freedOffset);
    }
}


} // namespace android
