#ifndef ANDROID_SURFACETEXTUREGLTOGL_H
#define ANDROID_SURFACETEXTUREGLTOGL_H


#include "SurfaceTextureGL.h"

namespace android {

// * This test fixture is for testing GL -> GL texture streaming.  It creates an
// * EGLSurface and an EGLContext for the image producer to use.
class SurfaceTextureGLToGLTest : public SurfaceTextureGLTest {
  protected:
    inline SurfaceTextureGLToGLTest() :
                mProducerEglSurface(EGL_NO_SURFACE),
                mProducerEglContext(EGL_NO_CONTEXT) {
        };

    inline virtual void SetUp() {
            SurfaceTextureGLTest::SetUp();
    
            mProducerEglSurface = eglCreateWindowSurface(mEglDisplay, mGlConfig,
                    mANW.get(), NULL);
            ASSERT_EQ(EGL_SUCCESS, eglGetError());
            ASSERT_NE(EGL_NO_SURFACE, mProducerEglSurface);
    
            mProducerEglContext = eglCreateContext(mEglDisplay, mGlConfig,
                    EGL_NO_CONTEXT, getContextAttribs());
            ASSERT_EQ(EGL_SUCCESS, eglGetError());
            ASSERT_NE(EGL_NO_CONTEXT, mProducerEglContext);
        };

    inline virtual void TearDown() {
            if (mProducerEglContext != EGL_NO_CONTEXT) {
                eglDestroyContext(mEglDisplay, mProducerEglContext);
            }
            if (mProducerEglSurface != EGL_NO_SURFACE) {
                eglDestroySurface(mEglDisplay, mProducerEglSurface);
            }
            SurfaceTextureGLTest::TearDown();
        };

    EGLSurface mProducerEglSurface;

    EGLContext mProducerEglContext;

};

} // namespace android
#endif
