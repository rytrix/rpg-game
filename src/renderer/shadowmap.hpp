#pragma once

#include "texture.hpp"
#include "framebuffer.hpp"

namespace Renderer {

class ShadowMap {
public:
    ShadowMap() = default;
    void init();
    void init(i32 width, i32 height);

    void bind();
    void unbind();

    i32 get_width();
    i32 get_height();

private:
    static constexpr i32 DEFAULT_SHADOW_WIDTH = 1024;
    static constexpr i32 DEFAULT_SHADOW_HEIGHT = 1024;

    i32 m_shadow_width = DEFAULT_SHADOW_WIDTH;
    i32 m_shadow_height = DEFAULT_SHADOW_HEIGHT;
    Renderer::Texture m_texture;
    Renderer::Framebuffer m_framebuffer;

    void init_internal();
};

} // namespace Renderer
