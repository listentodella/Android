#ifndef ANDROID_SURFACETEXTUREGL_H
#define ANDROID_SURFACETEXTUREGL_H


#include "GLTest.h"

namespace android { class TextureRenderer; } 
namespace android { class FrameWaiter; } 

namespace android {

class SurfaceTextureGLTest : public GLTest {
  public:
    enum  {
      TEX_ID =  123 

    };


  protected:
    inline void SetUp() {
            GLTest::SetUp();
            sp<IGraphicBufferProducer> producer;
            BufferQueue::createBufferQueue(&producer, &mConsumer);
            mST = new GLConsumer(mConsumer, TEX_ID, GLConsumer::TEXTURE_EXTERNAL,
                    true, false);
            mSTC = new Surface(producer);
            mANW = mSTC;
            mTextureRenderer = new TextureRenderer(TEX_ID, mST);
            ASSERT_NO_FATAL_FAILURE(mTextureRenderer->SetUp());
            mFW = new FrameWaiter;
            mST->setFrameAvailableListener(mFW);
        };

    inline void TearDown() {
            mTextureRenderer.clear();
            mANW.clear();
            mSTC.clear();
            mST.clear();
            GLTest::TearDown();
        };

    inline void drawTexture() {
            mTextureRenderer->drawTexture();
        };

    sp<IGraphicBufferConsumer> mConsumer;

    sp<GLConsumer> mST;

    sp<Surface> mSTC;

    sp<ANativeWindow> mANW;

    sp<TextureRenderer> mTextureRenderer;

    sp<FrameWaiter> mFW;

};

} // namespace android
#endif
