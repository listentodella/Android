#ifndef ANDROID_MULTITEXTURECONSUMERTEST_H
#define ANDROID_MULTITEXTURECONSUMERTEST_H


#include "GLTest.h"

namespace android {

class MultiTextureConsumerTest : public GLTest {
  public:
    enum  {
      TEX_ID =  123 

    };


  protected:
    inline virtual void SetUp() {
            GLTest::SetUp();
            sp<IGraphicBufferProducer> producer;
            sp<IGraphicBufferConsumer> consumer;
            BufferQueue::createBufferQueue(&producer, &consumer);
            mGlConsumer = new GLConsumer(consumer, TEX_ID,
                    GLConsumer::TEXTURE_EXTERNAL, true, false);
            mSurface = new Surface(producer);
            mANW = mSurface.get();
    
        };

    inline virtual void TearDown() {
            GLTest::TearDown();
        };

    inline virtual const EGLint * getContextAttribs() {
            return NULL;
        };

    inline virtual const EGLint * getConfigAttribs() {
            static EGLint sDefaultConfigAttribs[] = {
                EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
                EGL_RED_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_BLUE_SIZE, 8,
                EGL_ALPHA_SIZE, 8,
                EGL_NONE };
    
            return sDefaultConfigAttribs;
        };

    sp<GLConsumer> mGlConsumer;

    sp<Surface> mSurface;

    ANativeWindow * mANW;

};

} // namespace android
#endif
