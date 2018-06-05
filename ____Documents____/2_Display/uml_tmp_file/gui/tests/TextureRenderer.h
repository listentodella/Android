#ifndef ANDROID_TEXTURERENDERER_H
#define ANDROID_TEXTURERENDERER_H


namespace android {

typedef RefBase typedef6;
class TextureRenderer : public typedef6 {
  public:
    TextureRenderer(GLuint texName, const sp<GLConsumer> & st);

    void SetUp();

    void drawTexture();


  private:
    GLuint mTexName;

    sp<GLConsumer> mST;

    GLuint mPgm;

    GLint mPositionHandle;

    GLint mTexSamplerHandle;

    GLint mTexMatrixHandle;

};

} // namespace android
#endif
