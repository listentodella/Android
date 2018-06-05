#ifndef ANDROID_MULTISURFACETEXTURECLIENTTEST_H
#define ANDROID_MULTISURFACETEXTURECLIENTTEST_H


#include "GLTest.h"

namespace android {

class MultiSurfaceTextureClientTest : public typedef3 {
  public:
    inline MultiSurfaceTextureClientTest() :
                mEglDisplay(EGL_NO_DISPLAY),
                mEglContext(EGL_NO_CONTEXT) {
            for (int i = 0; i < NUM_SURFACE_TEXTURES; i++) {
                mEglSurfaces[i] = EGL_NO_CONTEXT;
            }
        };

    enum  {
      NUM_SURFACE_TEXTURES =  32 

    };


  protected:
    inline virtual void SetUp() {
            mEglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
            ASSERT_EQ(EGL_SUCCESS, eglGetError());
            ASSERT_NE(EGL_NO_DISPLAY, mEglDisplay);
    
            EGLint majorVersion, minorVersion;
            EXPECT_TRUE(eglInitialize(mEglDisplay, &majorVersion, &minorVersion));
            ASSERT_EQ(EGL_SUCCESS, eglGetError());
    
            EGLConfig myConfig;
            EGLint numConfigs = 0;
            EGLint configAttribs[] = {
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_NONE
            };
            EXPECT_TRUE(eglChooseConfig(mEglDisplay, configAttribs, &myConfig, 1,
                    &numConfigs));
            ASSERT_EQ(EGL_SUCCESS, eglGetError());
    
            mEglContext = eglCreateContext(mEglDisplay, myConfig, EGL_NO_CONTEXT,
                    0);
            ASSERT_EQ(EGL_SUCCESS, eglGetError());
            ASSERT_NE(EGL_NO_CONTEXT, mEglContext);
    
            for (int i = 0; i < NUM_SURFACE_TEXTURES; i++) {
                sp<IGraphicBufferProducer> producer;
                sp<IGraphicBufferConsumer> consumer;
                BufferQueue::createBufferQueue(&producer, &consumer);
                sp<GLConsumer> st(new GLConsumer(consumer, i,
                        GLConsumer::TEXTURE_EXTERNAL, true, false));
                sp<Surface> stc(new Surface(producer));
                mEglSurfaces[i] = eglCreateWindowSurface(mEglDisplay, myConfig,
                        static_cast<ANativeWindow*>(stc.get()), NULL);
                ASSERT_EQ(EGL_SUCCESS, eglGetError());
                ASSERT_NE(EGL_NO_SURFACE, mEglSurfaces[i]);
            }
        };

    inline virtual void TearDown() {
            eglMakeCurrent(mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE,
                    EGL_NO_CONTEXT);
    
            for (int i = 0; i < NUM_SURFACE_TEXTURES; i++) {
                if (mEglSurfaces[i] != EGL_NO_SURFACE) {
                    eglDestroySurface(mEglDisplay, mEglSurfaces[i]);
                }
            }
    
            if (mEglContext != EGL_NO_CONTEXT) {
                eglDestroyContext(mEglDisplay, mEglContext);
            }
    
            if (mEglDisplay != EGL_NO_DISPLAY) {
                eglTerminate(mEglDisplay);
            }
        };

    EGLDisplay mEglDisplay;

    EGLSurface mEglSurfaces[NUM_SURFACE_TEXTURES];

    EGLContext mEglContext;

};

} // namespace android
#endif
