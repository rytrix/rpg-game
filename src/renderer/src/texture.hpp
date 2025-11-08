#pragma once

namespace Renderer {

struct TextureSize {
    GLint width = 0;
    GLint height = 0;
    GLint depth = 0;
};

struct TextureInfo {
    bool fromFile = GL_FALSE;
    union {
        const char* file = nullptr;
        TextureSize size;
    };
    GLenum dimensions = GL_TEXTURE_2D;
    GLint min_filter = GL_LINEAR;
    GLint mag_filter = GL_LINEAR;
    GLint wrap_s = GL_REPEAT;
    GLint wrap_t = GL_REPEAT;
    bool mipmaps = GL_FALSE;
    GLenum internalFormat = GL_RGBA8;
};

struct TextureSubimageInfo {
    GLint level;
    TextureSize offsets;
    TextureSize size;
    GLenum format;
    GLenum type;
    void* pixels;
};

struct Texture {
    Texture();
    explicit Texture(TextureInfo* info);
    ~Texture();

    void init(TextureInfo* info);
    void subImage(TextureSubimageInfo* info);
    void bind(GLuint textureUnit);

    NODISCARD GLuint getId() const noexcept { return id; }

private:
    GLuint id;
    GLenum dimensions;

    void generateMipmap();
    void textureStorage(TextureSize* size, GLenum internalFormat);
    void fromFile(const char* file);
};

} // namespace Mcq
