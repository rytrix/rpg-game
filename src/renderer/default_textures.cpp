#include "default_textures.hpp"

#include "texture.hpp"

namespace Renderer {

DefaultTextures::DefaultTextures(TextureCache* cache)
{
    init(cache);
}

DefaultTextures::~DefaultTextures()
{
    if (m_cache != nullptr) {
        m_cache->remove(m_albedo);
        m_cache->remove(m_metallic);
        m_cache->remove(m_normal);
        m_cache = nullptr;
    }
    m_albedo = 0;
    m_metallic = 0;
    m_normal = 0;
    initialized = false;
}

void DefaultTextures::init(TextureCache* cache)
{
    TextureSize size = {
        .width = 1,
        .height = 1,
        .depth = 0,
    };
    TextureInfo texture_info;
    texture_info.size = size;
    texture_info.from_file = GL_FALSE;
    texture_info.mipmaps = false;
    texture_info.flip = false;
    texture_info.internal_format = GL_RGBA8;

    TextureSubimageInfo subimage_info;
    subimage_info.size = size;
    subimage_info.format = GL_RGBA;
    subimage_info.type = GL_UNSIGNED_BYTE;

    std::array<u8, 4> data_albedo = { 255, 255, 255, 255 };
    subimage_info.pixels = data_albedo.data();
    m_albedo = cache->add("DefaultTextureAlbedo", texture_info);
    cache->get(m_albedo)->sub_image(subimage_info);

    std::array<u8, 4> data_metallic = { 255, 0, 0, 0 };
    subimage_info.pixels = data_metallic.data();
    m_metallic = cache->add("DefaultTextureMetallic", texture_info);
    cache->get(m_metallic)->sub_image(subimage_info);

    subimage_info.format = GL_RGBA;
    subimage_info.type = GL_FLOAT;
    std::array<float, 4> data_normal = { 0.5F, 0.5F, 1.0F, 0.0F };
    subimage_info.pixels = data_normal.data();
    m_normal = cache->add("DefaultTextureNormal", texture_info);
    cache->get(m_normal)->sub_image(subimage_info);

    m_cache = cache;
    initialized = true;
}

u32 DefaultTextures::get_albedo() const
{
    util_assert(initialized == true, "DefaultTextures not initialized");
    return m_albedo;
}

u32 DefaultTextures::get_metallic() const
{
    util_assert(initialized == true, "DefaultTextures not initialized");
    return m_metallic;
}

u32 DefaultTextures::get_normal() const
{
    util_assert(initialized == true, "DefaultTextures not initialized");
    return m_normal;
}

} // namespace Renderer
