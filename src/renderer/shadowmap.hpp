#pragma once

#include "framebuffer.hpp"
#include "texture.hpp"
#include "shader.hpp"

namespace Renderer {

class ShadowMap {
public:
    ShadowMap() = default;
    void init();
    void init(i32 width, i32 height);

    void bind();
    void unbind();

    [[nodiscard]] i32 get_width() const;
    [[nodiscard]] i32 get_height() const;

    static consteval std::array<Renderer::ShaderInfo, 2> get_shader_info()
    {
        return std::array<Renderer::ShaderInfo, 2> {
            Renderer::ShaderInfo {
                .is_file = false,
                .shader = get_vertex_shader(),
                .type = GL_VERTEX_SHADER,
            },
            Renderer::ShaderInfo {
                .is_file = false,
                .shader = get_frag_shader(),
                .type = GL_FRAGMENT_SHADER,
            },
        };
    }

private:
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

    static constexpr i32 DEFAULT_SHADOW_WIDTH = 1024;
    static constexpr i32 DEFAULT_SHADOW_HEIGHT = 1024;

    i32 m_shadow_width = DEFAULT_SHADOW_WIDTH;
    i32 m_shadow_height = DEFAULT_SHADOW_HEIGHT;
    Renderer::Texture m_texture;
    Renderer::Framebuffer m_framebuffer;

    void init_internal();
};

} // namespace Renderer
