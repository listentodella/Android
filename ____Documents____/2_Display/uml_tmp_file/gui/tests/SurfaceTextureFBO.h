#ifndef ANDROID_SURFACETEXTUREFBO_H
#define ANDROID_SURFACETEXTUREFBO_H


#include "SurfaceTextureGL.h"

namespace android {

class SurfaceTextureFBOTest : public SurfaceTextureGLTest {
  protected:
    inline virtual void SetUp() {
            SurfaceTextureGLTest::SetUp();
    
            glGenFramebuffers(1, &mFbo);
            ASSERT_EQ(GLenum(GL_NO_ERROR), glGetError());
    
            glGenTextures(1, &mFboTex);
            glBindTexture(GL_TEXTURE_2D, mFboTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getSurfaceWidth(),
                    getSurfaceHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glBindTexture(GL_TEXTURE_2D, 0);
            ASSERT_EQ(GLenum(GL_NO_ERROR), glGetError());
    
            glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                    GL_TEXTURE_2D, mFboTex, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            ASSERT_EQ(GLenum(GL_NO_ERROR), glGetError());
        };

    inline virtual void TearDown() {
            SurfaceTextureGLTest::TearDown();
    
            glDeleteTextures(1, &mFboTex);
            glDeleteFramebuffers(1, &mFbo);
        };

    GLuint mFbo;

    GLuint mFboTex;

};

} // namespace android
#endif
