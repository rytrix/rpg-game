#include "shadowmap.hpp"

namespace Renderer {

void ShadowMap::init()
{
    init_internal();
}

void ShadowMap::init(i32 width, i32 height)
{
    m_shadow_width = width;
    m_shadow_height = height;
    init_internal();
}

void ShadowMap::init_internal()
{
    Renderer::TextureInfo shadowmap_info;
    shadowmap_info.size = Renderer::TextureSize { .width = m_shadow_width, .height = m_shadow_height, .depth = 0 };
    shadowmap_info.internal_format = GL_DEPTH_COMPONENT24;
    m_texture.init(shadowmap_info);

    m_framebuffer.init();
    m_framebuffer.bind_texture(GL_DEPTH_ATTACHMENT, m_texture.get_id(), 0);
    m_framebuffer.bind();
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    m_framebuffer.unbind();
}

void ShadowMap::bind()
{
    m_framebuffer.bind();
}

void ShadowMap::unbind()
{
    m_framebuffer.unbind();
}

i32 ShadowMap::get_width()
{
    return m_shadow_width;
}

i32 ShadowMap::get_height()
{
    return m_shadow_height;
}

} // namespace Renderer
