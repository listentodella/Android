#ifndef ANDROID_SURFACETEST_H
#define ANDROID_SURFACETEST_H


#include "GLTest.h"

namespace android {

class SurfaceTest : public typedef3 {
  protected:
    inline SurfaceTest() {
            ProcessState::self()->startThreadPool();
        };

    inline virtual void SetUp() {
            mComposerClient = new SurfaceComposerClient;
            ASSERT_EQ(NO_ERROR, mComposerClient->initCheck());
    
            mSurfaceControl = mComposerClient->createSurface(
                    String8("Test Surface"), 32, 32, PIXEL_FORMAT_RGBA_8888, 0);
    
            ASSERT_TRUE(mSurfaceControl != NULL);
            ASSERT_TRUE(mSurfaceControl->isValid());
    
            SurfaceComposerClient::openGlobalTransaction();
            ASSERT_EQ(NO_ERROR, mSurfaceControl->setLayer(0x7fffffff));
            ASSERT_EQ(NO_ERROR, mSurfaceControl->show());
            SurfaceComposerClient::closeGlobalTransaction();
    
            mSurface = mSurfaceControl->getSurface();
            ASSERT_TRUE(mSurface != NULL);
        };

    inline virtual void TearDown() {
            mComposerClient->dispose();
        };

    sp<Surface> mSurface;

    sp<SurfaceComposerClient> mComposerClient;

    sp<SurfaceControl> mSurfaceControl;

};

} // namespace android
#endif
