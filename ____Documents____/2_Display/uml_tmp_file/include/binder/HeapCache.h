#ifndef ANDROID_HEAPCACHE_H
#define ANDROID_HEAPCACHE_H


#include "typedef5.h"

class IBinder;
class IMemoryHeap;

namespace android {

// ---------------------------------------------------------------------------
class HeapCache : public typedef5 {
  public:
    HeapCache();

    virtual ~HeapCache();

    virtual void binderDied(const wp<IBinder> & binder);

    sp<IMemoryHeap> find_heap(const sp<IBinder> & binder);

    void free_heap(const sp<IBinder> & binder);

    sp<IMemoryHeap> get_heap(const sp<IBinder> & binder);

    void dump_heaps();

    struct heap_info_t {
        sp<IMemoryHeap> heap;

        int32_t count;

    };
    

  private:
    void free_heap(const wp<IBinder> & binder);

    Mutex mHeapCacheLock;

    KeyedVector<wp<IBinder>,heap_info_t> mHeapCache;

};

} // namespace android
#endif
