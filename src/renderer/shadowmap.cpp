#include "shadowmap.hpp"

namespace Renderer {

ShadowMap::~ShadowMap()
{
    initialized = false;
}

void ShadowMap::init()
{
    init_internal(false);
}

void ShadowMap::init(i32 width, i32 height)
{
    m_shadow_width = width;
    m_shadow_height = height;
    init_internal(false);
}

void ShadowMap::init_cubemap()
{
    init_internal(true);
}

void ShadowMap::init_cubemap(i32 width, i32 height)
{
    m_shadow_width = width;
    m_shadow_height = height;
    init_internal(true);
}

void ShadowMap::init_internal(bool cubemap)
{
    util_assert(initialized == false, "ShadowMap::init_internal() has already been initialized");

    Renderer::TextureInfo shadowmap_info;
    if (cubemap) {
        shadowmap_info.dimensions = GL_TEXTURE_CUBE_MAP;
    } else {
        shadowmap_info.dimensions = GL_TEXTURE_2D;
    }
    shadowmap_info.size = Renderer::TextureSize { .width = m_shadow_width, .height = m_shadow_height, .depth = 0 };
    shadowmap_info.internal_format = GL_DEPTH_COMPONENT24;
    shadowmap_info.wrap_s = GL_CLAMP_TO_BORDER;
    shadowmap_info.wrap_t = GL_CLAMP_TO_BORDER;
    shadowmap_info.wrap_r = GL_CLAMP_TO_BORDER;
    m_texture.init(shadowmap_info);

    m_framebuffer.init();
    m_framebuffer.bind_texture(GL_DEPTH_ATTACHMENT, m_texture.get_id(), 0);
    m_framebuffer.bind_draw_buffer(GL_NONE);
    m_framebuffer.bind_read_buffer(GL_NONE);

    initialized = true;
}

void ShadowMap::bind()
{
    util_assert(initialized == true, "ShadowMap has not been initialized");
    m_framebuffer.bind();
}

void ShadowMap::unbind()
{
    util_assert(initialized == true, "ShadowMap has not been initialized");
    m_framebuffer.unbind();
}

[[nodiscard]] i32 ShadowMap::get_width() const
{
    util_assert(initialized == true, "ShadowMap has not been initialized");
    return m_shadow_width;
}

[[nodiscard]] i32 ShadowMap::get_height() const
{
    util_assert(initialized == true, "ShadowMap has not been initialized");
    return m_shadow_height;
}

[[nodiscard]] Texture& ShadowMap::get_texture()
{
    util_assert(initialized == true, "ShadowMap has not been initialized");
    return m_texture;
}

[[nodiscard]] bool ShadowMap::is_initialized() const
{
    return initialized;
}

} // namespace Renderer
