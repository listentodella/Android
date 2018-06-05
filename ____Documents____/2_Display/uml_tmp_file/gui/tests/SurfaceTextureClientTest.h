#ifndef ANDROID_SURFACETEXTURECLIENTTEST_H
#define ANDROID_SURFACETEXTURECLIENTTEST_H


#include "GLTest.h"

namespace android {

class SurfaceTextureClientTest : public typedef3 {
  protected:
    inline SurfaceTextureClientTest() :
                mEglDisplay(EGL_NO_DISPLAY),
                mEglSurface(EGL_NO_SURFACE),
                mEglContext(EGL_NO_CONTEXT) {
        };

    inline virtual void SetUp() {
            const ::testing::TestInfo* const testInfo =
                ::testing::UnitTest::GetInstance()->current_test_info();
            ALOGV("Begin test: %s.%s", testInfo->test_case_name(),
                    testInfo->name());
    
            sp<IGraphicBufferProducer> producer;
            sp<IGraphicBufferConsumer> consumer;
            BufferQueue::createBufferQueue(&producer, &consumer);
            mST = new GLConsumer(consumer, 123, GLConsumer::TEXTURE_EXTERNAL, true,
                    false);
            mSTC = new Surface(producer);
            mANW = mSTC;
    
            // We need a valid GL context so we can test updateTexImage()
            // This initializes EGL and create a dummy GL context with a
            // pbuffer render target.
            mEglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
            ASSERT_EQ(EGL_SUCCESS, eglGetError());
            ASSERT_NE(EGL_NO_DISPLAY, mEglDisplay);
    
            EGLint majorVersion, minorVersion;
            EXPECT_TRUE(eglInitialize(mEglDisplay, &majorVersion, &minorVersion));
            ASSERT_EQ(EGL_SUCCESS, eglGetError());
    
            EGLConfig myConfig;
            EGLint numConfigs = 0;
            EXPECT_TRUE(eglChooseConfig(mEglDisplay, getConfigAttribs(),
                    &myConfig, 1, &numConfigs));
            ASSERT_EQ(EGL_SUCCESS, eglGetError());
    
            mEglConfig = myConfig;
            EGLint pbufferAttribs[] = {
                EGL_WIDTH, 16,
                EGL_HEIGHT, 16,
                EGL_NONE };
            mEglSurface = eglCreatePbufferSurface(mEglDisplay, myConfig, pbufferAttribs);
            ASSERT_EQ(EGL_SUCCESS, eglGetError());
            ASSERT_NE(EGL_NO_SURFACE, mEglSurface);
    
            mEglContext = eglCreateContext(mEglDisplay, myConfig, EGL_NO_CONTEXT, 0);
            ASSERT_EQ(EGL_SUCCESS, eglGetError());
            ASSERT_NE(EGL_NO_CONTEXT, mEglContext);
    
            EXPECT_TRUE(eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, mEglContext));
            ASSERT_EQ(EGL_SUCCESS, eglGetError());
        };

    inline virtual void TearDown() {
            mST.clear();
            mSTC.clear();
            mANW.clear();
    
            eglMakeCurrent(mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            eglDestroyContext(mEglDisplay, mEglContext);
            eglDestroySurface(mEglDisplay, mEglSurface);
            eglTerminate(mEglDisplay);
    
            const ::testing::TestInfo* const testInfo =
                ::testing::UnitTest::GetInstance()->current_test_info();
            ALOGV("End test:   %s.%s", testInfo->test_case_name(),
                    testInfo->name());
        };

    inline virtual const EGLint * getConfigAttribs() {
            static EGLint sDefaultConfigAttribs[] = {
                EGL_SURFACE_TYPE, EGL_PBUFFER_BIT | EGL_WINDOW_BIT,
                EGL_NONE
            };
    
            return sDefaultConfigAttribs;
        };

    sp<GLConsumer> mST;

    sp<Surface> mSTC;

    sp<ANativeWindow> mANW;

    EGLDisplay mEglDisplay;

    EGLSurface mEglSurface;

    EGLContext mEglContext;

    EGLConfig mEglConfig;

};

} // namespace android
#endif
