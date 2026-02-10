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
    GLint wrap_r = GL_REPEAT;
    std::array<float, 4> border_color = { 1.0F, 1.0F, 1.0F, 1.0F };
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

class Texture : public NoCopyNoMove {
public:
    Texture() = default;
    explicit Texture(TextureInfo& info);
    ~Texture();

    static GLuint get_texture_unit();
    static void reset_texture_units();

    void init(TextureInfo& info);
    void sub_image(TextureSubimageInfo& info);
    void bind(GLuint texture_unit);

    [[nodiscard]] GLuint64 get_bindless_texture_id();
    [[nodiscard]] bool is_bindless_texture_mapped();
    void map_bindless_texture();
    void unmap_bindless_texture();

    void set_max_anisotropy(float max_anisotropy);

    [[nodiscard]] GLuint get_id() const noexcept;

private:
    bool initialized = false;

    GLuint m_id {};
    GLenum m_dimensions {};

    bool m_bindless_texture_mapped = false;

    void generate_mipmap();
    void texture_storage(TextureSize& size, GLenum internal_format);
    void from_file(const char* file, bool flip);
};

} // namespace Renderer
