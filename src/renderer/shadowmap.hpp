#pragma once

#include "framebuffer.hpp"
#include "texture.hpp"

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

    static consteval const char* get_vertex_shader()
    {
        return R"(
            #version 460 core
            layout (location = 0) in vec3 aPos;

            uniform mat4 light_space_matrix;
            uniform mat4 model;

            void main()
            {
            gl_Position = light_space_matrix * model * vec4(aPos, 1.0);
            }
        )";
    }

    static consteval const char* get_frag_shader()
    {
        return R"(
            #version 460 core
            void main()
            {
            }
        )";
    }

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
