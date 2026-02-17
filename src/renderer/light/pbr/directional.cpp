#include "directional.hpp"

#include "../../../scene/shader_preprocessor.hpp"

#include <algorithm>

namespace Renderer::Light::Pbr {

void Directional::set_uniforms(Renderer::ShaderProgram& shader, const char* light_name) const
{
    shader.set_vec3(std::format("{}.direction", light_name).c_str(), direction);
    shader.set_vec3(std::format("{}.color", light_name).c_str(), color);
}

void DirectionalShadow::init()
{
    auto shader_text = get_directional_cascade_shader_text(m_cascades);
    std::array<Renderer::ShaderInfo, 3> shader_info {
        Renderer::ShaderInfo {
            .is_file = false,
            .shader = shader_text.at(0).c_str(),
            .type = GL_VERTEX_SHADER,
        },
        Renderer::ShaderInfo {
            .is_file = false,
            .shader = shader_text.at(1).c_str(),
            .type = GL_GEOMETRY_SHADER,
        },
        Renderer::ShaderInfo {
            .is_file = false,
            .shader = shader_text.at(2).c_str(),
            .type = GL_FRAGMENT_SHADER,
        },
    };

    m_shader.init(shader_info.data(), shader_info.size());
    m_shadowmap.init_cascade(2000, 2000, static_cast<int>(m_cascades));
    initialized = true;
}

DirectionalShadow::~DirectionalShadow()
{
    if (initialized) {
        initialized = false;
    }
}

void DirectionalShadow::update(const Directional& light, const Renderer::Camera& camera)
{
    util_assert(initialized == true, "Light::DirectionalShadow has not been initialized");
    util_assert(m_cascades <= MAX_CASCADES, "Light::DirectionalShadow cascades greater than max (16)");

    f32 near = camera.get_near();
    f32 far = camera.get_far();
    const f32 lambda = 0.85F;

    m_cascade_plane_distances.at(0) = near;
    m_cascade_plane_distances.at(m_cascades) = far;
    for (u32 i = 1; i < m_cascades + 1; i++) {
        f32 ratio = static_cast<f32>(i) / static_cast<f32>(m_cascades);
        f32 log_split = near * std::pow(far / near, ratio);
        f32 lin_split = near + ((far - near) * ratio);
        m_cascade_plane_distances.at(i) = glm::mix(lin_split, log_split, lambda);
    }

    for (u32 i = 0; i < m_cascades; i++) {
        glm::mat4 projection = glm::perspective(camera.get_fov(), camera.get_aspect(), m_cascade_plane_distances.at(i), m_cascade_plane_distances.at(i + 1));
        m_light_space_matrix.at(i) = calculate_light_space_matrix(light, projection, camera.get_view(), m_cascade_plane_distances.at(m_cascades));
    }
}

void DirectionalShadow::shadowmap_draw(const std::function<void(Renderer::ShaderProgram& shader)>& draw_function)
{
    util_assert(initialized == true, "Light::DirectionalShadow has not been initialized");

    glViewport(0, 0, m_shadowmap.get_width(), m_shadowmap.get_height());
    m_shadowmap.bind();

    glClear(GL_DEPTH_BUFFER_BIT);
    m_shader.bind();
    for (u32 i = 0; i < m_cascades; i++) {
        m_shader.set_mat4(std::format("light_space_matrices[{}]", i).c_str(), m_light_space_matrix.at(i));
    }
    draw_function(m_shader);

    m_shadowmap.unbind();

    // ImGui::Begin("Shadow Map Debug");
    // GLuint textureID = m_shadowmap.get_texture().get_id();
    // ImGui::Image((void*)(intptr_t)textureID, ImVec2(256, 256), ImVec2(0, 1), ImVec2(1, 0));
    // ImGui::End();
}

void DirectionalShadow::set_uniforms(Renderer::ShaderProgram& shader, const char* light_name)
{
    util_assert(initialized == true, "Light::DirectionalShadow has not been initialized");

    GLuint texture_unit = Texture::get_texture_unit();
    m_shadowmap.get_texture().bind(texture_unit);
    shader.set_int(std::format("{}.shadow_map", light_name).c_str(), static_cast<int>(texture_unit));
    for (u32 i = 0; i < m_cascades; i++) {
        shader.set_mat4(std::format("{}.light_space_matrix[{}]", light_name, i).c_str(), m_light_space_matrix.at(i));
        shader.set_float(std::format("{}.cascade_plane_distances[{}]", light_name, i).c_str(), m_cascade_plane_distances.at(i + 1));
    }
    shader.set_int(std::format("{}.cascade_count", light_name).c_str(), static_cast<int>(m_cascades));
}

glm::mat4 DirectionalShadow::calculate_light_space_matrix(const Directional& light, const glm::mat4 proj, const glm::mat4 view, f32 far)
{
    util_assert(initialized == true, "Light::DirectionalShadow has not been initialized");

    glm::mat4 inv_proj_view = glm::inverse(proj * view);

    std::array<glm::vec4, 8> frustum_corners;
    u32 i = 0;
    glm::vec3 center {};
    for (u32 x = 0; x < 2; ++x) {
        for (u32 y = 0; y < 2; ++y) {
            for (u32 z = 0; z < 2; ++z) {
                const glm::vec4 pt = inv_proj_view * glm::vec4(2.0F * x - 1.0F, 2.0F * y - 1.0F, 2.0F * z - 1.0F, 1.0F);
                frustum_corners.at(i) = (pt / pt.w);
                center += frustum_corners.at(i);
                i++;
            }
        }
    }
    center /= 8.0F;

    glm::mat4 light_view = glm::lookAt(
        center - light.direction,
        center,
        glm::vec3(0.0F, 1.0F, 0.0F));

    f32 radius = 0.0F;
    for (const auto& corner : frustum_corners) {
        f32 distance = glm::distance(center, glm::vec3(corner));
        radius = std::max(distance, radius);
    }

    // Texel Snapping
    f32 shadow_map_size = static_cast<f32>(m_shadowmap.get_width());
    f32 texel_size = (2.0F * radius) / shadow_map_size;
    glm::vec4 shadow_origin = glm::vec4(glm::vec3(0.0F), 1.0F);
    shadow_origin = light_view * shadow_origin;
    shadow_origin.x = glm::floor(shadow_origin.x / texel_size) * texel_size;
    shadow_origin.y = glm::floor(shadow_origin.y / texel_size) * texel_size;
    glm::vec3 offset = glm::vec3(shadow_origin) - glm::vec3(light_view * glm::vec4(glm::vec3(0.0F), 1.0F));
    light_view[3][0] += offset.x;
    light_view[3][1] += offset.y;

    glm::mat4 light_projection = glm::ortho(
        -radius,
        radius,
        -radius,
        radius,
        -far * 2.0F,
        far * 2.0F);

    return light_projection * light_view;
}

std::array<std::string, 3> DirectionalShadow::get_directional_cascade_shader_text(u32 cascade_count)
{
    std::array<std::string, 3> shaders;

    std::vector<char> shadow_file = read_file<char>("res/forward_pass/directional_cascades.glsl");
    std::string_view shadow_file_view = { shadow_file.data(), shadow_file.size() };

    // Vertex Shader
    shaders.at(0) = "#version 460 core\n";
    shaders.at(0) += get_lines_between_delims(shadow_file_view, "// Vertex Begin", "// Vertex End");

    // Geometry Shader
    shaders.at(1) = std::format("#version 460 core\n#define CASCADE_COUNT {}\n", cascade_count);
    shaders.at(1) += get_lines_between_delims(shadow_file_view, "// Geometry Begin", "// Geometry End");

    // Fragment Shader
    shaders.at(2) += "#version 460 core\n";
    shaders.at(2) += get_lines_between_delims(shadow_file_view, "// Fragment Begin", "// Fragment End");

    return shaders;
}

} // Renderer::Light::Pbr