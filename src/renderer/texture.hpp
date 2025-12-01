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
        const char* file_path = nullptr;
        TextureSize size;
    };
    GLenum dimensions = GL_TEXTURE_2D;
    GLint min_filter = GL_NEAREST;
    GLint mag_filter = GL_NEAREST;
    GLint wrap_s = GL_REPEAT;
    GLint wrap_t = GL_REPEAT;
    bool mipmaps = GL_TRUE;
    GLenum internal_format = GL_RGBA8;
    bool flip = true;
};

struct TextureSubimageInfo {
    GLint level {};
    TextureSize offsets {};
    TextureSize size {};
    GLenum format {};
    GLenum type {};
    void* pixels {};
};

class Texture {
public:
    Texture() = default;
    explicit Texture(TextureInfo& info);
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&&) = default;
    Texture& operator=(Texture&&) = default;

    void init(TextureInfo& info);
    void sub_image(TextureSubimageInfo& info);
    void bind(GLuint texture_unit);

    [[nodiscard]] GLuint get_id() const noexcept;

private:
    GLuint m_id {};
    GLenum m_dimensions {};

    void generate_mipmap();
    void texture_storage(TextureSize& size, GLenum internal_format);
    void from_file(const char* file, bool flip);
};

struct TextureStorage {
    Texture m_tex;
    std::string file_path;
};

struct TextureRef {
    TextureStorage* m_tex;
    u32 m_id;
    const char* m_type;
};

} // namespace Renderer
