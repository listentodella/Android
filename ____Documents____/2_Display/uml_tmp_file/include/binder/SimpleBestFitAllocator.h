#ifndef ANDROID_SIMPLEBESTFITALLOCATOR_H
#define ANDROID_SIMPLEBESTFITALLOCATOR_H


#include "LinkedList.h"

namespace android {

// ----------------------------------------------------------------------------
class SimpleBestFitAllocator {
  public:
    enum  {
      PAGE_ALIGNED =  0x00000001
    

    };

    SimpleBestFitAllocator(size_t size);

    ~SimpleBestFitAllocator();

    size_t allocate(size_t size, uint32_t flags = 0);

    status_t deallocate(size_t offset);

    size_t size() const;

    void dump(const char * what) const;

    void dump(String8 & result, const char * what) const;

    struct chunk_t {
        inline chunk_t(size_t start, size_t size) : start(start), size(size), free(1), prev(0), next(0) {
                };

        size_t start;

        size_t size : 28;

        int free : 4;

        mutable chunk_t * prev;

        mutable chunk_t * next;

    };
    

  private:
    ssize_t alloc(size_t size, uint32_t flags);

    chunk_t * dealloc(size_t start);

    void dump_l(const char * what) const;

    void dump_l(String8 & result, const char * what) const;

    static const int kMemoryAlign;

    mutable Mutex mLock;

    LinkedList<chunk_t> mList;

    size_t mHeapSize;

};

} // namespace android
#endif
