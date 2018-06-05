#ifndef ANDROID_COMPOSER_H
#define ANDROID_COMPOSER_H


#include "typedef17.h"

namespace android {

class Composer : public typedef17 {

    friend class Singleton<Composer>;
  private:
    mutable Mutex mLock;

    SortedVector<ComposerState> mComposerStates;

    SortedVector<DisplayState > mDisplayStates;

    uint32_t mForceSynchronous;

    uint32_t mTransactionNestCount;

    bool mAnimation;

    inline Composer() : Singleton<Composer>(),
            mForceSynchronous(0), mTransactionNestCount(0),
            mAnimation(false)
        { };

    void openGlobalTransactionImpl();

    void closeGlobalTransactionImpl(bool synchronous);

    void setAnimationTransactionImpl();

    layer_state_t * getLayerStateLocked(const sp<SurfaceComposerClient> & client, const sp<IBinder> & id);

    DisplayState & getDisplayStateLocked(const sp<IBinder> & token);


  public:
    sp<IBinder> createDisplay(const String8 & displayName, bool secure);

    void destroyDisplay(const sp<IBinder> & display);

    sp<IBinder> getBuiltInDisplay(int32_t id);

    status_t setPosition(const sp<SurfaceComposerClient> & client, const sp<IBinder> & id, float x, float y);

    status_t setSize(const sp<SurfaceComposerClient> & client, const sp<IBinder> & id, uint32_t w, uint32_t h);

    status_t setLayer(const sp<SurfaceComposerClient> & client, const sp<IBinder> & id, int32_t z);

    status_t setFlags(const sp<SurfaceComposerClient> & client, const sp<IBinder> & id, uint32_t flags, uint32_t mask);

    status_t setTransparentRegionHint(const sp<SurfaceComposerClient> & client, const sp<IBinder> & id, const Region & transparentRegion);

    status_t setAlpha(const sp<SurfaceComposerClient> & client, const sp<IBinder> & id, float alpha);

    status_t setMatrix(const sp<SurfaceComposerClient> & client, const sp<IBinder> & id, float dsdx, float dtdx, float dsdy, float dtdy);

    status_t setOrientation(int orientation);

    status_t setCrop(const sp<SurfaceComposerClient> & client, const sp<IBinder> & id, const Rect & crop);

    status_t setLayerStack(const sp<SurfaceComposerClient> & client, const sp<IBinder> & id, uint32_t layerStack);

    void setDisplaySurface(const sp<IBinder> & token, const sp<IGraphicBufferProducer> & bufferProducer);

    void setDisplayLayerStack(const sp<IBinder> & token, uint32_t layerStack);

    void setDisplayProjection(const sp<IBinder> & token, uint32_t orientation, const Rect & layerStackRect, const Rect & displayRect);

    void setDisplaySize(const sp<IBinder> & token, uint32_t width, uint32_t height);

    static inline void setAnimationTransaction() {
            Composer::getInstance().setAnimationTransactionImpl();
        };

    static inline void openGlobalTransaction() {
            Composer::getInstance().openGlobalTransactionImpl();
        };

    static inline void closeGlobalTransaction(bool synchronous) {
            Composer::getInstance().closeGlobalTransactionImpl(synchronous);
        };

};

} // namespace android
#endif
