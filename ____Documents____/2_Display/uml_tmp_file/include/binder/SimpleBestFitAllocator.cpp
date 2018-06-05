
#include "SimpleBestFitAllocator.h"

namespace android {

SimpleBestFitAllocator::SimpleBestFitAllocator(size_t size){
    size_t pagesize = getpagesize();
    mHeapSize = ((size + pagesize-1) & ~(pagesize-1));

    chunk_t* node = new chunk_t(0, mHeapSize / kMemoryAlign);
    mList.insertHead(node);
}

SimpleBestFitAllocator::~SimpleBestFitAllocator(){
    while(!mList.isEmpty()) {
        delete mList.remove(mList.head());
    }
}

size_t SimpleBestFitAllocator::allocate(size_t size, uint32_t flags) {
    Mutex::Autolock _l(mLock);
    ssize_t offset = alloc(size, flags);
    return offset;
}

status_t SimpleBestFitAllocator::deallocate(size_t offset) {
    Mutex::Autolock _l(mLock);
    chunk_t const * const freed = dealloc(offset);
    if (freed) {
        return NO_ERROR;
    }
    return NAME_NOT_FOUND;
}

size_t SimpleBestFitAllocator::size() const {
    return mHeapSize;
}

void SimpleBestFitAllocator::dump(const char * what) const {
    Mutex::Autolock _l(mLock);
    dump_l(what);
}

void SimpleBestFitAllocator::dump(String8 & result, const char * what) const {
    Mutex::Autolock _l(mLock);
    dump_l(result, what);
}

ssize_t SimpleBestFitAllocator::alloc(size_t size, uint32_t flags) {
    if (size == 0) {
        return 0;
    }
    size = (size + kMemoryAlign-1) / kMemoryAlign;
    chunk_t* free_chunk = 0;
    chunk_t* cur = mList.head();

    size_t pagesize = getpagesize();
    while (cur) {
        int extra = 0;
        if (flags & PAGE_ALIGNED)
            extra = ( -cur->start & ((pagesize/kMemoryAlign)-1) ) ;

        // best fit
        if (cur->free && (cur->size >= (size+extra))) {
            if ((!free_chunk) || (cur->size < free_chunk->size)) {
                free_chunk = cur;
            }
            if (cur->size == size) {
                break;
            }
        }
        cur = cur->next;
    }

    if (free_chunk) {
        const size_t free_size = free_chunk->size;
        free_chunk->free = 0;
        free_chunk->size = size;
        if (free_size > size) {
            int extra = 0;
            if (flags & PAGE_ALIGNED)
                extra = ( -free_chunk->start & ((pagesize/kMemoryAlign)-1) ) ;
            if (extra) {
                chunk_t* split = new chunk_t(free_chunk->start, extra);
                free_chunk->start += extra;
                mList.insertBefore(free_chunk, split);
            }

            ALOGE_IF((flags&PAGE_ALIGNED) && 
                    ((free_chunk->start*kMemoryAlign)&(pagesize-1)),
                    "PAGE_ALIGNED requested, but page is not aligned!!!");

            const ssize_t tail_free = free_size - (size+extra);
            if (tail_free > 0) {
                chunk_t* split = new chunk_t(
                        free_chunk->start + free_chunk->size, tail_free);
                mList.insertAfter(free_chunk, split);
            }
        }
        return (free_chunk->start)*kMemoryAlign;
    }
    return NO_MEMORY;
}

SimpleBestFitAllocator::chunk_t * SimpleBestFitAllocator::dealloc(size_t start) {
    start = start / kMemoryAlign;
    chunk_t* cur = mList.head();
    while (cur) {
        if (cur->start == start) {
            LOG_FATAL_IF(cur->free,
                "block at offset 0x%08lX of size 0x%08lX already freed",
                cur->start*kMemoryAlign, cur->size*kMemoryAlign);

            // merge freed blocks together
            chunk_t* freed = cur;
            cur->free = 1;
            do {
                chunk_t* const p = cur->prev;
                chunk_t* const n = cur->next;
                if (p && (p->free || !cur->size)) {
                    freed = p;
                    p->size += cur->size;
                    mList.remove(cur);
                    delete cur;
                }
                cur = n;
            } while (cur && cur->free);

            #ifndef NDEBUG
                if (!freed->free) {
                    dump_l("dealloc (!freed->free)");
                }
            #endif
            LOG_FATAL_IF(!freed->free,
                "freed block at offset 0x%08lX of size 0x%08lX is not free!",
                freed->start * kMemoryAlign, freed->size * kMemoryAlign);

            return freed;
        }
        cur = cur->next;
    }
    return 0;
}

void SimpleBestFitAllocator::dump_l(const char * what) const {
    String8 result;
    dump_l(result, what);
    ALOGD("%s", result.string());
}

void SimpleBestFitAllocator::dump_l(String8 & result, const char * what) const {
    size_t size = 0;
    int32_t i = 0;
    chunk_t const* cur = mList.head();
    
    const size_t SIZE = 256;
    char buffer[SIZE];
    snprintf(buffer, SIZE, "  %s (%p, size=%u)\n",
            what, this, (unsigned int)mHeapSize);
    
    result.append(buffer);
            
    while (cur) {
        const char* errs[] = {"", "| link bogus NP",
                            "| link bogus PN", "| link bogus NP+PN" };
        int np = ((cur->next) && cur->next->prev != cur) ? 1 : 0;
        int pn = ((cur->prev) && cur->prev->next != cur) ? 2 : 0;

        snprintf(buffer, SIZE, "  %3u: %p | 0x%08X | 0x%08X | %s %s\n",
            i, cur, int(cur->start*kMemoryAlign),
            int(cur->size*kMemoryAlign),
                    int(cur->free) ? "F" : "A",
                    errs[np|pn]);
        
        result.append(buffer);

        if (!cur->free)
            size += cur->size*kMemoryAlign;

        i++;
        cur = cur->next;
    }
    snprintf(buffer, SIZE,
            "  size allocated: %u (%u KB)\n", int(size), int(size/1024));
    result.append(buffer);
}

const int SimpleBestFitAllocator::kMemoryAlign= 32;


} // namespace android
