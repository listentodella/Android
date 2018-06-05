#ifndef ANDROID_SURFACETEXTUREGLTHREADTOGL_H
#define ANDROID_SURFACETEXTUREGLTHREADTOGL_H


#include "SurfaceTextureGLToGL.h"

namespace android {

// * This test fixture is for testing GL -> GL texture streaming from one thread
// * to another.  It contains functionality to create a producer thread that will
// * perform GL rendering to an ANativeWindow that feeds frames to a
// * GLConsumer.  Additionally it supports interlocking the producer and
// * consumer threads so that a specific sequence of calls can be
// * deterministically created by the test.
// *
// * The intended usage is as follows:
// *
// * TEST_F(...) {
// *     class PT : public ProducerThread {
// *         virtual void render() {
// *             ...
// *             swapBuffers();
// *         }
// *     };
// *
// *     runProducerThread(new PT());
// *
// *     // The order of these calls will vary from test to test and may include
// *     // multiple frames and additional operations (e.g. GL rendering from the
// *     // texture).
// *     fc->waitForFrame();
// *     mST->updateTexImage();
// *     fc->finishFrame();
// * }
// *
class SurfaceTextureGLThreadToGLTest : public SurfaceTextureGLToGLTest {
  public:
    class ProducerThread : public typedef4 {
      public:
        inline virtual ~ProducerThread() {
                };

        inline void setEglObjects(EGLDisplay producerEglDisplay, EGLSurface producerEglSurface, EGLContext producerEglContext) {
                    mProducerEglDisplay = producerEglDisplay;
                    mProducerEglSurface = producerEglSurface;
                    mProducerEglContext = producerEglContext;
                };

        inline virtual bool threadLoop() {
                    eglMakeCurrent(mProducerEglDisplay, mProducerEglSurface,
                            mProducerEglSurface, mProducerEglContext);
                    render();
                    eglMakeCurrent(mProducerEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE,
                            EGL_NO_CONTEXT);
                    return false;
                };


      protected:
        virtual void render() = 0;

        inline void swapBuffers() {
                    eglSwapBuffers(mProducerEglDisplay, mProducerEglSurface);
                };

        EGLDisplay mProducerEglDisplay;

        EGLSurface mProducerEglSurface;

        EGLContext mProducerEglContext;

    };
    
    typedef Thread typedef4;

    class FrameCondition : public typedef5 {
      public:
        inline FrameCondition() :
                        mFrameAvailable(false),
                        mFrameFinished(false) {
                };

        // waitForFrame waits for the next frame to arrive.  This should be
        // called from the consumer thread once for every frame expected by the
        // test.
        inline void waitForFrame() {
                    Mutex::Autolock lock(mMutex);
                    ALOGV("+waitForFrame");
                    while (!mFrameAvailable) {
                        mFrameAvailableCondition.wait(mMutex);
                    }
                    mFrameAvailable = false;
                    ALOGV("-waitForFrame");
                };

        // Allow the producer to return from its swapBuffers call and continue
        // on to produce the next frame.  This should be called by the consumer
        // thread once for every frame expected by the test.
        inline void finishFrame() {
                    Mutex::Autolock lock(mMutex);
                    ALOGV("+finishFrame");
                    mFrameFinished = true;
                    mFrameFinishCondition.signal();
                    ALOGV("-finishFrame");
                };

        // This should be called by GLConsumer on the producer thread.
        inline virtual void onFrameAvailable() {
                    Mutex::Autolock lock(mMutex);
                    ALOGV("+onFrameAvailable");
                    mFrameAvailable = true;
                    mFrameAvailableCondition.signal();
                    while (!mFrameFinished) {
                        mFrameFinishCondition.wait(mMutex);
                    }
                    mFrameFinished = false;
                    ALOGV("-onFrameAvailable");
                };


      protected:
        bool mFrameAvailable;

        bool mFrameFinished;

        Mutex mMutex;

        Condition mFrameAvailableCondition;

        Condition mFrameFinishCondition;

    };
    
    typedef GLConsumer::FrameAvailableListener typedef5;


  protected:
    inline virtual void SetUp() {
            SurfaceTextureGLToGLTest::SetUp();
            mFC = new FrameCondition();
            mST->setFrameAvailableListener(mFC);
        };

    inline virtual void TearDown() {
            if (mProducerThread != NULL) {
                mProducerThread->requestExitAndWait();
            }
            mProducerThread.clear();
            mFC.clear();
            SurfaceTextureGLToGLTest::TearDown();
        };

    inline void runProducerThread(const sp<ProducerThread> producerThread) {
            ASSERT_TRUE(mProducerThread == NULL);
            mProducerThread = producerThread;
            producerThread->setEglObjects(mEglDisplay, mProducerEglSurface,
                    mProducerEglContext);
            producerThread->run();
        };

    sp<ProducerThread> mProducerThread;

    sp<FrameCondition> mFC;

};

} // namespace android
#endif
