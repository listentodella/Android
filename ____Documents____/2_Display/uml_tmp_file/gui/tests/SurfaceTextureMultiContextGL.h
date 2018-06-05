#ifndef ANDROID_SURFACETEXTUREMULTICONTEXTGL_H
#define ANDROID_SURFACETEXTUREMULTICONTEXTGL_H


#include "SurfaceTextureGL.h"

namespace android { class TextureRenderer; } 

namespace android {

class SurfaceTextureMultiContextGLTest : public SurfaceTextureGLTest {
  public:
    enum  {
      SECOND_TEX_ID =  123 

    };

    enum  {
      THIRD_TEX_ID =  456 

    };


  protected:
    inline SurfaceTextureMultiContextGLTest() :
                mSecondEglContext(EGL_NO_CONTEXT) {
        };

    inline virtual void SetUp() {
            SurfaceTextureGLTest::SetUp();
    
            // Set up the secondary context and texture renderer.
            mSecondEglContext = eglCreateContext(mEglDisplay, mGlConfig,
                    EGL_NO_CONTEXT, getContextAttribs());
            ASSERT_EQ(EGL_SUCCESS, eglGetError());
            ASSERT_NE(EGL_NO_CONTEXT, mSecondEglContext);
    
            ASSERT_TRUE(eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface,
                    mSecondEglContext));
            ASSERT_EQ(EGL_SUCCESS, eglGetError());
            mSecondTextureRenderer = new TextureRenderer(SECOND_TEX_ID, mST);
            ASSERT_NO_FATAL_FAILURE(mSecondTextureRenderer->SetUp());
    
            // Set up the tertiary context and texture renderer.
            mThirdEglContext = eglCreateContext(mEglDisplay, mGlConfig,
                    EGL_NO_CONTEXT, getContextAttribs());
            ASSERT_EQ(EGL_SUCCESS, eglGetError());
            ASSERT_NE(EGL_NO_CONTEXT, mThirdEglContext);
    
            ASSERT_TRUE(eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface,
                    mThirdEglContext));
            ASSERT_EQ(EGL_SUCCESS, eglGetError());
            mThirdTextureRenderer = new TextureRenderer(THIRD_TEX_ID, mST);
            ASSERT_NO_FATAL_FAILURE(mThirdTextureRenderer->SetUp());
    
            // Switch back to the primary context to start the tests.
            ASSERT_TRUE(eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface,
                    mEglContext));
        };

    inline virtual void TearDown() {
            if (mThirdEglContext != EGL_NO_CONTEXT) {
                eglDestroyContext(mEglDisplay, mThirdEglContext);
            }
            if (mSecondEglContext != EGL_NO_CONTEXT) {
                eglDestroyContext(mEglDisplay, mSecondEglContext);
            }
            SurfaceTextureGLTest::TearDown();
        };

    EGLContext mSecondEglContext;

    sp<TextureRenderer> mSecondTextureRenderer;

    EGLContext mThirdEglContext;

    sp<TextureRenderer> mThirdTextureRenderer;

};

} // namespace android
#endif
