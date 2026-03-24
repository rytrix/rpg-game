#pragma once

#include "../renderer/shader.hpp"
#include "shader_preprocessor.hpp"

namespace {

template <usize S>
struct ShaderInfoData {
    std::array<std::string, S> data;
    std::array<Renderer::ShaderInfo, S> info;

    void populate_info()
    {
        if constexpr (S == 2) {
            info.at(0) = {
                .is_file = false,
                .shader = data.at(0).c_str(),
                .type = GL_VERTEX_SHADER,
            };
            info.at(1) = {
                .is_file = false,
                .shader = data.at(1).c_str(),
                .type = GL_FRAGMENT_SHADER,
            };
        } else if constexpr (S == 3) {
            info.at(0) = {
                .is_file = false,
                .shader = data.at(0).c_str(),
                .type = GL_VERTEX_SHADER,
            };
            info.at(1) = {
                .is_file = false,
                .shader = data.at(1).c_str(),
                .type = GL_GEOMETRY_SHADER,
            };
            info.at(2) = {
                .is_file = false,
                .shader = data.at(2).c_str(),
                .type = GL_FRAGMENT_SHADER,
            };
        }
    }
};

constexpr void get_pbr_forward_pass_indirect(ShaderInfoData<2>& out, const std::string& light_uniforms, const std::string& light_functions, const std::string& vert_defines, const std::string& frag_defines)
{
    std::vector<char> pbr_file = read_file<char>("res/shaders/forward_pass/pbr_combined.glsl");
    std::string_view pbr_file_view = { pbr_file.data(), pbr_file.size() };

    // Vertex Shader
    out.data.at(0) = std::format("#version 460 core\n#define SSBO0\n");
    out.data.at(0) += vert_defines;
    out.data.at(0) += get_lines_between_delims(pbr_file_view, "// Vertex Begin", "// Vertex End");

    // Fragment Shader
    out.data.at(1) += "#version 460 core\n#define BINDLESS_TEXTURES\n";
    out.data.at(1) += frag_defines;
    out.data.at(1) += get_lines_between_delims(pbr_file_view, "// Fragment Begin", "// Light Uniforms Begin");
    out.data.at(1) += light_uniforms;
    out.data.at(1) += get_lines_between_delims(pbr_file_view, "// Light Uniforms End", "// LO Functions Begin");
    out.data.at(1) += light_functions;
    out.data.at(1) += get_lines_between_delims(pbr_file_view, "// LO Functions End", "// Fragment End");

    out.populate_info();
}

constexpr void get_pbr_forward_pass_normal(ShaderInfoData<2>& out, const std::string& light_uniforms, const std::string& light_functions, const std::string& vert_defines, const std::string& frag_defines)
{
    std::vector<char> pbr_file = read_file<char>("res/shaders/forward_pass/pbr_combined.glsl");
    std::string_view pbr_file_view = { pbr_file.data(), pbr_file.size() };

    // Vertex Shader
    out.data.at(0) = std::format("#version 460 core\n#define SSBO0\n");
    out.data.at(0) += vert_defines;
    out.data.at(0) += get_lines_between_delims(pbr_file_view, "// Vertex Begin", "// Vertex End");

    // Fragment Shader
    out.data.at(1) += "#version 460 core\n#define UNIFORM_TEXTURES\n";
    out.data.at(1) += frag_defines;
    out.data.at(1) += get_lines_between_delims(pbr_file_view, "// Fragment Begin", "// Light Uniforms Begin");
    out.data.at(1) += light_uniforms;
    out.data.at(1) += get_lines_between_delims(pbr_file_view, "// Light Uniforms End", "// LO Functions Begin");
    out.data.at(1) += light_functions;
    out.data.at(1) += get_lines_between_delims(pbr_file_view, "// LO Functions End", "// Fragment End");

    out.populate_info();
}

constexpr void get_shadow_pass_basic_shaders(ShaderInfoData<2>& out, const std::string& vert_defines, const std::string& frag_defines)
{
    std::vector<char> shadow_file = read_file<char>("res/shaders/forward_pass/shadow_pass_basic.glsl");
    std::string_view shadow_file_view = { shadow_file.data(), shadow_file.size() };

    // Vertex Shader
    out.data.at(0) = "#version 460 core\n";
    out.data.at(0) += vert_defines;
    out.data.at(0) += get_lines_between_delims(shadow_file_view, "// Vertex Begin", "// Vertex End");

    // Fragment Shader
    out.data.at(1) += "#version 460 core\n";
    out.data.at(1) += frag_defines;
    out.data.at(1) += get_lines_between_delims(shadow_file_view, "// Fragment Begin", "// Fragment End");

    out.populate_info();
}

constexpr void get_directional_cascade_shader(ShaderInfoData<2>& out, const std::string& vert_defines, const std::string& frag_defines)
{
    std::vector<char> shadow_file = read_file<char>("res/shaders/forward_pass/shadow_pass_directional_cascades.glsl");
    std::string_view shadow_file_view = { shadow_file.data(), shadow_file.size() };

    // Vertex Shader
    out.data.at(0) = "#version 460 core\n";
    out.data.at(0) += vert_defines;
    out.data.at(0) += get_lines_between_delims(shadow_file_view, "// Vertex Begin", "// Vertex End");

    // Fragment Shader
    out.data.at(1) += "#version 460 core\n";
    out.data.at(1) += frag_defines;
    out.data.at(1) += get_lines_between_delims(shadow_file_view, "// Fragment Begin", "// Fragment End");

    out.populate_info();
}

constexpr void get_directional_cascade_shader_geometry(ShaderInfoData<3>& out, const std::string& vert_defines, const std::string& frag_defines, u32 cascade_count)
{
    std::vector<char> shadow_file = read_file<char>("res/shaders/forward_pass/shadow_pass_directional_cascades_geometry.glsl");
    std::string_view shadow_file_view = { shadow_file.data(), shadow_file.size() };

    // Vertex Shader
    out.data.at(0) = "#version 460 core\n";
    out.data.at(0) += vert_defines;
    out.data.at(0) += get_lines_between_delims(shadow_file_view, "// Vertex Begin", "// Vertex End");

    // Geometry Shader
    out.data.at(1) = std::format("#version 460 core\n#define CASCADE_COUNT {}\n", cascade_count);
    out.data.at(1) += get_lines_between_delims(shadow_file_view, "// Geometry Begin", "// Geometry End");

    // Fragment Shader
    out.data.at(2) += "#version 460 core\n";
    out.data.at(2) += frag_defines;
    out.data.at(2) += get_lines_between_delims(shadow_file_view, "// Fragment Begin", "// Fragment End");

    out.populate_info();
}

constexpr void get_shadow_pass_point_shaders(ShaderInfoData<2>& out, const std::string& vert_defines, const std::string& frag_defines)
{
    std::vector<char> shadow_file = read_file<char>("res/shaders/forward_pass/shadow_pass_point.glsl");
    std::string_view shadow_file_view = { shadow_file.data(), shadow_file.size() };

    // Vertex Shader
    out.data.at(0) = "#version 460 core\n";
    out.data.at(0) += vert_defines;
    out.data.at(0) += get_lines_between_delims(shadow_file_view, "// Vertex Begin", "// Vertex End");

    // Fragment Shader
    out.data.at(1) += "#version 460 core\n";
    out.data.at(1) += frag_defines;
    out.data.at(1) += get_lines_between_delims(shadow_file_view, "// Fragment Begin", "// Fragment End");

    out.populate_info();
}

constexpr void get_shadow_pass_point_geometry_shaders(ShaderInfoData<3>& out, const std::string& vert_defines, const std::string& frag_defines)
{
    std::vector<char> shadow_file = read_file<char>("res/shaders/forward_pass/shadow_pass_point_geometry.glsl");
    std::string_view shadow_file_view = { shadow_file.data(), shadow_file.size() };

    // Vertex Shader
    out.data.at(0) = "#version 460 core\n";
    out.data.at(0) += vert_defines;
    out.data.at(0) += get_lines_between_delims(shadow_file_view, "// Vertex Begin", "// Vertex End");

    // Geometry Shader
    out.data.at(1) = "#version 460 core\n";
    out.data.at(1) += get_lines_between_delims(shadow_file_view, "// Geometry Begin", "// Geometry End");

    // Fragment Shader
    out.data.at(2) += "#version 460 core\n";
    out.data.at(2) += frag_defines;
    out.data.at(2) += get_lines_between_delims(shadow_file_view, "// Fragment Begin", "// Fragment End");

    out.populate_info();
}

// #include "scene_shaders_pbr.hpp"
// #include "scene_shaders_phong.hpp"

} // anonymous namespace
