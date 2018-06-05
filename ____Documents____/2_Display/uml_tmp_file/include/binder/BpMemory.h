#ifndef ANDROID_BPMEMORY_H
#define ANDROID_BPMEMORY_H


#include "typedef7.h"

class IBinder;
class IMemoryHeap;

namespace android {

class BpMemory : public typedef7 {
  public:
    BpMemory(const sp<IBinder> & impl);

    virtual ~BpMemory();

    virtual sp<IMemoryHeap> getMemory(ssize_t * offset = 0, size_t * size = 0) const;


  private:
    mutable sp<IMemoryHeap> mHeap;

    mutable ssize_t mOffset;

    mutable size_t mSize;

};

} // namespace android
#endif
