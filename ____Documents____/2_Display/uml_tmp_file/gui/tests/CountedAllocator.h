#ifndef ANDROID_COUNTEDALLOCATOR_H
#define ANDROID_COUNTEDALLOCATOR_H


#include "typedef21.h"

namespace android {

class CountedAllocator : public typedef21 {
  public:
    inline CountedAllocator() : mAllocCount(0) {
            sp<ISurfaceComposer> composer(ComposerService::getComposerService());
            mAllocator = composer->createGraphicBufferAlloc();
        };

    inline virtual ~CountedAllocator() {};

    inline virtual sp<GraphicBuffer> createGraphicBuffer(uint32_t w, uint32_t h, PixelFormat format, uint32_t usage, status_t * error) {
            ++mAllocCount;
            sp<GraphicBuffer> buffer = mAllocator->createGraphicBuffer(w, h, format,
                    usage, error);
            return buffer;
        };

    inline int getAllocCount() const { return mAllocCount; };


  private:
    sp<IGraphicBufferAlloc> mAllocator;

    int mAllocCount;

};

} // namespace android
#endif
