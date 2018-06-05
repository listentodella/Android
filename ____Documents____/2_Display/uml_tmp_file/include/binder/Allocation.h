#ifndef ANDROID_ALLOCATION_H
#define ANDROID_ALLOCATION_H


#include "typedef10.h"

class MemoryDealer;
class IMemoryHeap;

namespace android {

// ----------------------------------------------------------------------------
class Allocation : public typedef10 {
  public:
    Allocation(const sp<MemoryDealer> & dealer, const sp<IMemoryHeap> & heap, ssize_t offset, size_t size);

    virtual ~Allocation();


  private:
    sp<MemoryDealer> mDealer;

};

} // namespace android
#endif
