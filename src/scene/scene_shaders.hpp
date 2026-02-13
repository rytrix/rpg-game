#pragma once

#include "shader_preprocessor.hpp"

namespace {

constexpr std::pair<std::string, std::string> get_pbr_forward_pass_indirect(const std::string& light_uniforms, const std::string& light_functions)
{
    std::pair<std::string, std::string> shaders;

    std::vector<char> pbr_file = read_file<char>("res/forward_pass/pbr_combined.glsl");
    std::string_view pbr_file_view = { pbr_file.data(), pbr_file.size() };

    // Vertex Shader
    std::string vertex_shader = get_lines_between_delims(pbr_file_view, "// Vertex Begin", "// Vertex End");
    ShaderPreprocessor vertex_processor(vertex_shader, { "SSBO0" });
    shaders.first = vertex_processor.process();

    // Fragment Shader
    std::string fragment_shader = get_lines_between_delims_inclusive(pbr_file_view, "// Fragment Begin", "// Fragment End");
    ShaderPreprocessor fragment_processor(fragment_shader, { "BindlessTextures" });
    fragment_shader = fragment_processor.process();
    shaders.second += get_lines_between_delims(fragment_shader, "// Fragment Begin", "// Light Uniforms Begin");
    shaders.second += light_uniforms;
    shaders.second += get_lines_between_delims(fragment_shader, "// Light Uniforms End", "// LO Functions Begin");
    shaders.second += light_functions;
    shaders.second += get_lines_between_delims(fragment_shader, "// LO Functions End", "// Fragment End");

    return shaders;
}

constexpr std::pair<std::string, std::string> get_pbr_forward_pass_normal(const std::string& light_uniforms, const std::string& light_functions)
{
    std::pair<std::string, std::string> shaders;

    std::vector<char> pbr_file = read_file<char>("res/forward_pass/pbr_combined.glsl");
    std::string_view pbr_file_view = { pbr_file.data(), pbr_file.size() };

    // Vertex Shader
    std::string vertex_shader = get_lines_between_delims(pbr_file_view, "// Vertex Begin", "// Vertex End");
    ShaderPreprocessor vertex_processor(vertex_shader, { "SSBO0" });
    shaders.first = vertex_processor.process();

    // Fragment Shader
    std::string fragment_shader = get_lines_between_delims_inclusive(pbr_file_view, "// Fragment Begin", "// Fragment End");
    ShaderPreprocessor fragment_processor(fragment_shader, { "UniformTextures" });
    fragment_shader = fragment_processor.process();
    shaders.second += get_lines_between_delims(fragment_shader, "// Fragment Begin", "// Light Uniforms Begin");
    shaders.second += light_uniforms;
    shaders.second += get_lines_between_delims(fragment_shader, "// Light Uniforms End", "// LO Functions Begin");
    shaders.second += light_functions;
    shaders.second += get_lines_between_delims(fragment_shader, "// LO Functions End", "// Fragment End");

    return shaders;
}

// #include "scene_shaders_pbr.hpp"
// #include "scene_shaders_phong.hpp"

} // anonymous namespace