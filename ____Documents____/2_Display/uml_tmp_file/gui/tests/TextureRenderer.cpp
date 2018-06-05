
#include "TextureRenderer.h"

namespace android {

TextureRenderer::TextureRenderer(GLuint texName, const sp<GLConsumer> & st) : mTexName(texName), mST(st) {
}

void TextureRenderer::SetUp() {
    const char vsrc[] =
        "attribute vec4 vPosition;\n"
        "varying vec2 texCoords;\n"
        "uniform mat4 texMatrix;\n"
        "void main() {\n"
        "  vec2 vTexCoords = 0.5 * (vPosition.xy + vec2(1.0, 1.0));\n"
        "  texCoords = (texMatrix * vec4(vTexCoords, 0.0, 1.0)).xy;\n"
        "  gl_Position = vPosition;\n"
        "}\n";

    const char fsrc[] =
        "#extension GL_OES_EGL_image_external : require\n"
        "precision mediump float;\n"
        "uniform samplerExternalOES texSampler;\n"
        "varying vec2 texCoords;\n"
        "void main() {\n"
        "  gl_FragColor = texture2D(texSampler, texCoords);\n"
        "}\n";

    {
        SCOPED_TRACE("creating shader program");
        ASSERT_NO_FATAL_FAILURE(GLTest::createProgram(vsrc, fsrc, &mPgm));
    }

    mPositionHandle = glGetAttribLocation(mPgm, "vPosition");
    ASSERT_EQ(GLenum(GL_NO_ERROR), glGetError());
    ASSERT_NE(-1, mPositionHandle);
    mTexSamplerHandle = glGetUniformLocation(mPgm, "texSampler");
    ASSERT_EQ(GLenum(GL_NO_ERROR), glGetError());
    ASSERT_NE(-1, mTexSamplerHandle);
    mTexMatrixHandle = glGetUniformLocation(mPgm, "texMatrix");
    ASSERT_EQ(GLenum(GL_NO_ERROR), glGetError());
    ASSERT_NE(-1, mTexMatrixHandle);
}

// drawTexture draws the GLConsumer over the entire GL viewport.

void TextureRenderer::drawTexture() {
    static const GLfloat triangleVertices[] = {
        -1.0f, 1.0f,
        -1.0f, -1.0f,
        1.0f, -1.0f,
        1.0f, 1.0f,
    };

    glVertexAttribPointer(mPositionHandle, 2, GL_FLOAT, GL_FALSE, 0,
            triangleVertices);
    ASSERT_EQ(GLenum(GL_NO_ERROR), glGetError());
    glEnableVertexAttribArray(mPositionHandle);
    ASSERT_EQ(GLenum(GL_NO_ERROR), glGetError());

    glUseProgram(mPgm);
    glUniform1i(mTexSamplerHandle, 0);
    ASSERT_EQ(GLenum(GL_NO_ERROR), glGetError());
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, mTexName);
    ASSERT_EQ(GLenum(GL_NO_ERROR), glGetError());

    // XXX: These calls are not needed for GL_TEXTURE_EXTERNAL_OES as
    // they're setting the defautls for that target, but when hacking
    // things to use GL_TEXTURE_2D they are needed to achieve the same
    // behavior.
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER,
            GL_LINEAR);
    ASSERT_EQ(GLenum(GL_NO_ERROR), glGetError());
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER,
            GL_LINEAR);
    ASSERT_EQ(GLenum(GL_NO_ERROR), glGetError());
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S,
            GL_CLAMP_TO_EDGE);
    ASSERT_EQ(GLenum(GL_NO_ERROR), glGetError());
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T,
            GL_CLAMP_TO_EDGE);
    ASSERT_EQ(GLenum(GL_NO_ERROR), glGetError());

    GLfloat texMatrix[16];
    mST->getTransformMatrix(texMatrix);
    glUniformMatrix4fv(mTexMatrixHandle, 1, GL_FALSE, texMatrix);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    ASSERT_EQ(GLenum(GL_NO_ERROR), glGetError());
}


} // namespace android
