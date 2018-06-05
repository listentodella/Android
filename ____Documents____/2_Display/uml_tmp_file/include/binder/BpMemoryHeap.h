#ifndef ANDROID_BPMEMORYHEAP_H
#define ANDROID_BPMEMORYHEAP_H


#include "typedef6.h"

class IBinder;
class IMemoryHeap;

namespace android {

class BpMemoryHeap : public typedef6 {
  public:
    BpMemoryHeap(const sp<IBinder> & impl);

    virtual ~BpMemoryHeap();

    virtual int getHeapID() const;

    virtual void * getBase() const;

    virtual size_t getSize() const;

    virtual uint32_t getFlags() const;

    virtual uint32_t getOffset() const;


    friend class IMemory;  friend class HeapCache;

  private:
    // for debugging in this module
    static inline sp<IMemoryHeap> find_heap(const sp<IBinder> & binder) {
            return gHeapCache->find_heap(binder);
        };

    static inline void free_heap(const sp<IBinder> & binder) {
            gHeapCache->free_heap(binder);
        };

    static inline sp<IMemoryHeap> get_heap(const sp<IBinder> & binder) {
            return gHeapCache->get_heap(binder);
        };

    static inline void dump_heaps() {
            gHeapCache->dump_heaps();
        };

    void assertMapped() const;

    void assertReallyMapped() const;

    mutable volatile int32_t mHeapId;

    mutable void * mBase;

    mutable size_t mSize;

    mutable uint32_t mFlags;

    mutable uint32_t mOffset;

    mutable bool mRealHeap;

    mutable Mutex mLock;

};

} // namespace android
#endif
