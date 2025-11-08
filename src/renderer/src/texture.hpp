#pragma once

namespace Renderer {

struct TextureSize {
    GLint width = 0;
    GLint height = 0;
    GLint depth = 0;
};

struct TextureInfo {
    bool from_file = GL_FALSE;
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
    GLenum internal_format = GL_RGBA8;
};

struct TextureSubimageInfo {
    GLint level {};
    TextureSize offsets {};
    TextureSize size {};
    GLenum format {};
    GLenum type {};
    void* pixels {};
};

struct Texture {
    Texture() = default;
    explicit Texture(TextureInfo* info);
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&&) = default;
    Texture& operator=(Texture&&) = default;

    void init(TextureInfo* info);
    void sub_image(TextureSubimageInfo* info);
    void bind(GLuint textureUnit);

    [[nodiscard]] GLuint get_id() const noexcept { return m_id; }

private:
    GLuint m_id {};
    GLenum m_dimensions {};

    void generate_mipmap();
    void texture_storage(TextureSize* size, GLenum internal_format);
    void from_file(const char* file);
};

} // namespace Mcq
