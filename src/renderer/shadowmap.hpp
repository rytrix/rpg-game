#pragma once

#include "framebuffer.hpp"
#include "shader.hpp"
#include "texture.hpp"

namespace Renderer {

class ShadowMap {
public:
    ShadowMap() = default;
    ~ShadowMap();

    ShadowMap(const ShadowMap&) = delete;
    ShadowMap& operator=(const ShadowMap&) = delete;
    ShadowMap(ShadowMap&&) = default;
    ShadowMap& operator=(ShadowMap&&) = default;

    void init();
    void init(i32 width, i32 height);
    void init_cubemap();
    void init_cubemap(i32 width, i32 height);

    void bind();
    void unbind();

    [[nodiscard]] i32 get_width() const;
    [[nodiscard]] i32 get_height() const;
    [[nodiscard]] Texture& get_texture();

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

    static consteval std::array<Renderer::ShaderInfo, 3> get_shader_info_cubemap()
    {
        return std::array<Renderer::ShaderInfo, 3> {
            Renderer::ShaderInfo {
                .is_file = false,
                .shader = get_vertex_shader_cubemap(),
                .type = GL_VERTEX_SHADER,
            },
            Renderer::ShaderInfo {
                .is_file = false,
                .shader = get_geometry_shader_cubemap(),
                .type = GL_GEOMETRY_SHADER,
            },
            Renderer::ShaderInfo {
                .is_file = false,
                .shader = get_frag_shader_cubemap(),
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

    static consteval const char* get_vertex_shader_cubemap()
    {
        return R"(
            #version 460 core
            layout (location = 0) in vec3 aPos;

            uniform mat4 model;

            void main()
            {
                gl_Position = model * vec4(aPos, 1.0);
            }
        )";
    }

    static consteval const char* get_geometry_shader_cubemap()
    {
        return R"(
        #version 460 core
        layout (triangles) in;
        layout (triangle_strip, max_vertices=18) out;

        uniform mat4 light_space_matrices[6];

        out vec4 FragPos; // FragPos from GS (output per emitvertex)

        void main()
        {
            for(int face = 0; face < 6; face++)
            {
                gl_Layer = face; // built-in variable that specifies to which face we render.
                for(int i = 0; i < 3; i++) // for each triangle vertex
                {
                    FragPos = gl_in[i].gl_Position;
                    gl_Position = light_space_matrices[face] * FragPos;
                    EmitVertex();
                }    
                EndPrimitive();
            }
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

    static consteval const char* get_frag_shader_cubemap()
    {
        return R"(
            #version 460 core
            in vec4 FragPos;

            uniform vec3 light_pos;
            uniform float far_plane;

            void main()
            {
                float light_distance = length(FragPos.xyz - light_pos);

                // map to [0;1] range
                light_distance = light_distance / far_plane;

                gl_FragDepth = light_distance;
            }
        )";
    }

    static constexpr i32 DEFAULT_SHADOW_WIDTH = 1024;
    static constexpr i32 DEFAULT_SHADOW_HEIGHT = 1024;

    bool initialized = false;

    i32 m_shadow_width = DEFAULT_SHADOW_WIDTH;
    i32 m_shadow_height = DEFAULT_SHADOW_HEIGHT;
    Renderer::Texture m_texture;
    Renderer::Framebuffer m_framebuffer;

    void init_internal(bool cubemap);
};

} // namespace Renderer
