#include "texture.hpp"

#include "extensions.hpp"

namespace {

class TextureAllocator {
public:
    explicit TextureAllocator(u32 max_textures);

    u32 next();
    void reset();

private:
    const u32 m_max_textures {};
    u32 m_front {};
};

TextureAllocator::TextureAllocator(u32 max_textures)
    : m_max_textures(max_textures)
{
}

u32 TextureAllocator::next()
{
    util_assert(m_front < m_max_textures,
        "TextureAllocator: attempting to call next when front is greater than max_textures");

    if (m_front >= m_max_textures) {
        reset();
    }

    LOG_TRACE(std::format("TextureAllocator::next() returning {}", m_front));

    return m_front++;
}

void TextureAllocator::reset()
{
    m_front = 0;
}

std::unique_ptr<TextureAllocator> create_texture_allocator()
{
    int max_units {};
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max_units);
    LOG_INFO(std::format("Max texture units {}", max_units));

    return std::make_unique<TextureAllocator>(static_cast<u32>(max_units));
}

std::unique_ptr<TextureAllocator> texture_unit_allocator = nullptr;

}

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

GLuint Texture::get_texture_unit()
{
    if (texture_unit_allocator == nullptr) {
        texture_unit_allocator = create_texture_allocator();
    }
    return texture_unit_allocator->next();
}

void Texture::reset_texture_units()
{
    if (texture_unit_allocator != nullptr) {
        texture_unit_allocator->reset();
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

[[nodiscard]] GLuint64 Texture::get_bindless_texture_id()
{
    util_assert(initialized == true, "Texture has not been initialized");
    util_assert(Renderer::Extensions::is_extension_supported("GL_ARB_bindless_texture") == true, "GL_ARB_bindless_texture not supported");
    return glGetTextureHandleARB(m_id);
}

[[nodiscard]] bool Texture::is_bindless_texture_mapped()
{
    util_assert(initialized == true, "Texture has not been initialized");
    util_assert(Renderer::Extensions::is_extension_supported("GL_ARB_bindless_texture") == true, "GL_ARB_bindless_texture not supported");
    return m_bindless_texture_mapped;
}

void Texture::map_bindless_texture()
{
    util_assert(initialized == true, "Texture has not been initialized");
    util_assert(Renderer::Extensions::is_extension_supported("GL_ARB_bindless_texture") == true, "GL_ARB_bindless_texture not supported");
    util_assert(m_bindless_texture_mapped == false, "Attempting to map bindless texture that is already mapped");
    glMakeTextureHandleResidentARB(get_bindless_texture_id());
    m_bindless_texture_mapped = true;
}

void Texture::unmap_bindless_texture()
{
    util_assert(m_bindless_texture_mapped == true, "Attempting to unmap bindless texture that is not mapped");
    util_assert(Renderer::Extensions::is_extension_supported("GL_ARB_bindless_texture") == true, "GL_ARB_bindless_texture not supported");
    glMakeTextureHandleNonResidentARB(get_bindless_texture_id());
    m_bindless_texture_mapped = false;
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
