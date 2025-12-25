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
    if (initialized) {
        throw std::runtime_error("ShadowMap::init_internal() attempting to reinit shadowmap");
    }

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
    m_framebuffer.bind();
}

void ShadowMap::unbind()
{
    m_framebuffer.unbind();
}

[[nodiscard]] i32 ShadowMap::get_width() const
{
    return m_shadow_width;
}

[[nodiscard]] i32 ShadowMap::get_height() const
{
    return m_shadow_height;
}

[[nodiscard]] Texture& ShadowMap::get_texture()
{
    return m_texture;
}

} // namespace Renderer
