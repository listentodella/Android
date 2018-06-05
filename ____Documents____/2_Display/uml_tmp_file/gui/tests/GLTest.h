#ifndef ANDROID_GLTEST_H
#define ANDROID_GLTEST_H


namespace android {

typedef ::testing::Test typedef3;
class GLTest : public typedef3 {
  public:
    static void loadShader(GLenum shaderType, const char * pSource, GLuint * outShader);

    static void createProgram(const char * pVertexSource, const char * pFragmentSource, GLuint * outPgm);


  protected:
    inline GLTest() :
                mEglDisplay(EGL_NO_DISPLAY),
                mEglSurface(EGL_NO_SURFACE),
                mEglContext(EGL_NO_CONTEXT) {
        };

    virtual void SetUp();

    virtual void TearDown();

    virtual const EGLint * getConfigAttribs();

    virtual const EGLint * getContextAttribs();

    virtual EGLint getSurfaceWidth();

    virtual EGLint getSurfaceHeight();

    virtual EGLSurface createWindowSurface(EGLDisplay display, EGLConfig config, sp<ANativeWindow> & window) const;

    ::testing::AssertionResult checkPixel(int x, int y, int r, int g, int b, int a, int tolerance = 2);

    ::testing::AssertionResult assertRectEq(const Rect & r1, const Rect & r2, int tolerance = 1);

    int mDisplaySecs;

    sp<SurfaceComposerClient> mComposerClient;

    sp<SurfaceControl> mSurfaceControl;

    EGLDisplay mEglDisplay;

    EGLSurface mEglSurface;

    EGLContext mEglContext;

    EGLConfig mGlConfig;

};

} // namespace android
#endif
