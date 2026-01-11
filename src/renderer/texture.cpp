#include "texture.hpp"

namespace Renderer {

Texture::Texture(TextureInfo& info)
{
    init(info);
}

Texture::~Texture()
{
    // util_assert(initialized == true, "Texture::~Texture() has not been initialized");
    if (initialized) {
        glDeleteTextures(1, &m_id);
        initialized = false;
    }
}

void Texture::init(TextureInfo& info)
{
    util_assert(initialized == false, "Texture::Init() has already been initialized");

    m_dimensions = info.dimensions;

    glCreateTextures(m_dimensions, 1, &m_id);

    glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, info.min_filter);
    glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, info.mag_filter);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, info.wrap_s);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, info.wrap_t);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_R, info.wrap_r);
    glTextureParameterfv(m_id, GL_TEXTURE_BORDER_COLOR, info.border_color.data());

    initialized = true;

    if (info.from_file) {
        from_file(info.file_path, info.flip);
    } else {
        texture_storage(info.size, info.internal_format);
    }

    if (info.mipmaps) {
        generate_mipmap();
    }
}

void Texture::generate_mipmap()
{
    util_assert(initialized == true, "Texture has not been initialized");
    glGenerateTextureMipmap(m_id);
}

void Texture::sub_image(TextureSubimageInfo& info)
{
    util_assert(initialized == true, "Texture has not been initialized");
    switch (m_dimensions) {
        case GL_TEXTURE_1D:
            glTextureSubImage1D(m_id,
                info.level,
                info.offsets.width,
                info.size.width,
                info.format,
                info.type,
                info.pixels);
            break;
        case GL_TEXTURE_2D:
            glTextureSubImage2D(m_id,
                info.level,
                info.offsets.width,
                info.offsets.height,
                info.size.width,
                info.size.height,
                info.format,
                info.type,
                info.pixels);
            break;
        case GL_TEXTURE_3D:
            glTextureSubImage3D(m_id,
                info.level,
                info.offsets.width,
                info.offsets.height,
                info.offsets.depth,
                info.size.width,
                info.size.height,
                info.size.depth,
                info.format,
                info.type,
                info.pixels);
            break;
        default:
            throw std::runtime_error(std::format("Texture::sub_image: invalid texture dimensions {}\n", m_dimensions));
    }
}

void Texture::bind(GLuint texture_unit)
{
    util_assert(initialized == true, "Texture has not been initialized");
    glBindTextureUnit(texture_unit, m_id);
}

[[nodiscard]] GLuint Texture::get_id() const noexcept
{
    util_assert(initialized == true, "Texture has not been initialized");
    return m_id;
}

void Texture::texture_storage(TextureSize& size, GLenum internal_format)
{
    util_assert(initialized == true, "Texture has not been initialized");
    switch (m_dimensions) {
        case GL_TEXTURE_1D:
            glTextureStorage1D(m_id, 1, internal_format, size.width);
            break;
        case GL_TEXTURE_2D:
            glTextureStorage2D(m_id, 1, internal_format, size.width, size.height);
            break;
        case GL_TEXTURE_3D:
            glTextureStorage3D(m_id, 1, internal_format, size.width, size.height, size.depth);
            break;
        case GL_TEXTURE_CUBE_MAP:
            glTextureStorage2D(m_id, 1, internal_format, size.width, size.height);
            break;
        default:
            throw std::runtime_error(std::format("Texture::texture_storage: invalid texture dimensions {}\n", m_dimensions));
    }
}

void Texture::from_file(const char* file, bool flip)
{
    util_assert(initialized == true, "Texture::from_file has not been initialized");

    if (m_dimensions != GL_TEXTURE_2D) {
        throw std::runtime_error("currently only 2D textures are supported from files");
    }

    if (flip) {
        stbi_set_flip_vertically_on_load(1);
    }

    TextureSize size {};
    int nr_channels {};
    unsigned char* data = stbi_load(file, &size.width, &size.height, &nr_channels, 0);
    if (data == nullptr) {
        throw std::runtime_error(std::format("failed to load texture {}", file));
    }

    TextureSubimageInfo info {};
    info.type = GL_UNSIGNED_BYTE;
    info.size = size;
    info.pixels = data;

    if (nr_channels == 3) {
        texture_storage(size, GL_RGB8);
        info.format = GL_RGB;
    } else if (nr_channels == 4) {
        texture_storage(size, GL_RGBA8);
        info.format = GL_RGBA;
    } else {
        throw std::runtime_error(std::format("Texture: invalid number of channels \"{}\"", nr_channels));
    }

    sub_image(info);

    stbi_image_free(data);
}

} // namespace Renderer
