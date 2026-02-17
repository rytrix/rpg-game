#pragma once

#include "shader_preprocessor.hpp"

namespace {

constexpr std::pair<std::string, std::string> get_pbr_forward_pass_indirect(const std::string& light_uniforms, const std::string& light_functions)
{
    std::pair<std::string, std::string> shaders;

    std::vector<char> pbr_file = read_file<char>("res/forward_pass/pbr_combined.glsl");
    std::string_view pbr_file_view = { pbr_file.data(), pbr_file.size() };

    // Vertex Shader
    shaders.first = "#version 460 core\n#define SSBO0\n";
    shaders.first += get_lines_between_delims(pbr_file_view, "// Vertex Begin", "// Vertex End");

    // Fragment Shader
    shaders.second += "#version 460 core\n#define BindlessTextures\n";
    shaders.second += get_lines_between_delims(pbr_file_view, "// Fragment Begin", "// Light Uniforms Begin");
    shaders.second += light_uniforms;
    shaders.second += get_lines_between_delims(pbr_file_view, "// Light Uniforms End", "// LO Functions Begin");
    shaders.second += light_functions;
    shaders.second += get_lines_between_delims(pbr_file_view, "// LO Functions End", "// Fragment End");

    return shaders;
}

constexpr std::pair<std::string, std::string> get_pbr_forward_pass_normal(const std::string& light_uniforms, const std::string& light_functions)
{
    std::pair<std::string, std::string> shaders;

    std::vector<char> pbr_file = read_file<char>("res/forward_pass/pbr_combined.glsl");
    std::string_view pbr_file_view = { pbr_file.data(), pbr_file.size() };

    // Vertex Shader
    shaders.first = "#version 460 core\n#define SSBO0\n";
    shaders.first += get_lines_between_delims(pbr_file_view, "// Vertex Begin", "// Vertex End");

    // Fragment Shader
    shaders.second += "#version 460 core\n#define UniformTextures\n";
    shaders.second += get_lines_between_delims(pbr_file_view, "// Fragment Begin", "// Light Uniforms Begin");
    shaders.second += light_uniforms;
    shaders.second += get_lines_between_delims(pbr_file_view, "// Light Uniforms End", "// LO Functions Begin");
    shaders.second += light_functions;
    shaders.second += get_lines_between_delims(pbr_file_view, "// LO Functions End", "// Fragment End");

    return shaders;
}

constexpr std::array<std::string, 2> get_shadow_pass_basic_shaders()
{
    std::array<std::string, 2> shaders;

    std::vector<char> shadow_file = read_file<char>("res/forward_pass/shadow_pass_basic.glsl");
    std::string_view shadow_file_view = { shadow_file.data(), shadow_file.size() };

    // Vertex Shader
    shaders.at(0) = "#version 460 core\n";
    shaders.at(0) += get_lines_between_delims(shadow_file_view, "// Vertex Begin", "// Vertex End");

    // Fragment Shader
    shaders.at(1) += "#version 460 core\n";
    shaders.at(1) += get_lines_between_delims(shadow_file_view, "// Fragment Begin", "// Fragment End");

    return shaders;
}

constexpr std::array<std::string, 2> get_shadow_pass_point_shaders()
{
    std::array<std::string, 2> shaders;

    std::vector<char> shadow_file = read_file<char>("res/forward_pass/shadow_pass_point.glsl");
    std::string_view shadow_file_view = { shadow_file.data(), shadow_file.size() };

    // Vertex Shader
    shaders.at(0) = "#version 460 core\n";
    shaders.at(0) += get_lines_between_delims(shadow_file_view, "// Vertex Begin", "// Vertex End");

    // Fragment Shader
    shaders.at(1) += "#version 460 core\n";
    shaders.at(1) += get_lines_between_delims(shadow_file_view, "// Fragment Begin", "// Fragment End");

    return shaders;
}

constexpr std::array<std::string, 3> get_shadow_pass_point_geometry_shaders()
{
    std::array<std::string, 3> shaders;

    std::vector<char> shadow_file = read_file<char>("res/forward_pass/shadow_pass_point_geometry.glsl");
    std::string_view shadow_file_view = { shadow_file.data(), shadow_file.size() };

    // Vertex Shader
    shaders.at(0) = "#version 460 core\n";
    shaders.at(0) += get_lines_between_delims(shadow_file_view, "// Vertex Begin", "// Vertex End");

    // Geometry Shader
    shaders.at(1) = "#version 460 core\n";
    shaders.at(1) += get_lines_between_delims(shadow_file_view, "// Geometry Begin", "// Geometry End");

    // Fragment Shader
    shaders.at(2) += "#version 460 core\n";
    shaders.at(2) += get_lines_between_delims(shadow_file_view, "// Fragment Begin", "// Fragment End");

    return shaders;
}

// #include "scene_shaders_pbr.hpp"
// #include "scene_shaders_phong.hpp"

} // anonymous namespace