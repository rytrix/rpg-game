#include "directional.hpp"

#include <algorithm>

namespace Renderer::Light::Pbr {

void Directional::set_uniforms(Renderer::ShaderProgram& shader, const char* light_name) const
{
    shader.set_vec3(std::format("{}.direction", light_name).c_str(), direction);
    shader.set_vec3(std::format("{}.color", light_name).c_str(), color);
}

void DirectionalShadow::init()
{
    m_shadowmap.init(4096, 4096);
    initialized = true;
}

DirectionalShadow::~DirectionalShadow()
{
    if (initialized) {
        initialized = false;
    }
}

void DirectionalShadow::update(const Directional& light, Renderer::Camera& camera)
{
    util_assert(initialized == true, "Light::DirectionalShadow has not been initialized");

    glm::mat4 inv_proj_view = camera.get_inverse_proj_view();

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

    // f32 radius = 0.0F;
    // for (usize i = 0; i < frustum_corners.size(); i++) {
    //     f32 distance = glm::distance(center, frustum_corners.at(i));
    //     radius = std::max(distance, radius);
    // }
    // // radius /= 1.0F;

    // // glm::vec3 normal = glm::normalize(camera.get_pos() - center);
    // glm::vec3 normal = glm::normalize(-light.direction);

    // constexpr f32 FAR_MULTIPLIER = 2.0F;
    // // Position where the shadow is cast from
    // glm::vec3 pos { center + normal * radius / FAR_MULTIPLIER };

    // // radius /= 2.0F;
    // glm::mat4 light_projection = glm::ortho(
    //     -radius,
    //     radius,
    //     -radius,
    //     radius,
    //     -Z_FAR,
    //     Z_FAR);

    // // light_projection = glm::perspective(
    // //     glm::radians(camera.get_fov()),
    // //     camera.get_aspect(),
    // //     0.1F,
    // //     100.0F);

    // glm::mat4 light_view = glm::lookAt(
    //     pos,
    //     center,
    //     glm::vec3(0.0F, 1.0F, 0.0F));


    glm::mat4 light_view = glm::lookAt(
        center - light.direction,
        center,
        glm::vec3(0.0F, 1.0F, 0.0F));

    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();
    for (const auto& corner : frustum_corners) {
        const auto trf = light_view * corner;
        minX = std::min(minX, trf.x);
        maxX = std::max(maxX, trf.x);
        minY = std::min(minY, trf.y);
        maxY = std::max(maxY, trf.y);
        minZ = std::min(minZ, trf.z);
        maxZ = std::max(maxZ, trf.z);
    }

    // Tune this parameter according to the scene
    constexpr float z_mult = 10.0f;
    if (minZ < 0) {
        minZ *= z_mult;
    } else {
        minZ /= z_mult;
    }
    if (maxZ < 0) {
        maxZ /= z_mult;
    } else {
        maxZ *= z_mult;
    }

    // // Texel Snapping
    // f32 shadow_map_size = static_cast<f32>(m_shadowmap.get_width());
    // f32 texel_size = (2.0F * something) / shadow_map_size;
    // glm::vec4 shadow_origin = glm::vec4(glm::vec3(0.0F), 1.0F);
    // shadow_origin = light_view * shadow_origin;
    // shadow_origin.x = glm::floor(shadow_origin.x / texel_size) * texel_size;
    // shadow_origin.y = glm::floor(shadow_origin.y / texel_size) * texel_size;
    // glm::vec3 offset = glm::vec3(shadow_origin) - glm::vec3(light_view * glm::vec4(glm::vec3(0.0F), 1.0F));
    // light_view[3][0] += offset.x;
    // light_view[3][1] += offset.y;

    const glm::mat4 light_projection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);

    m_light_space_matrix = light_projection * light_view;

    // if (!ImGui::Begin("Temp Debug", nullptr, 0)) {
    //     // Early out if the window is collapsed, as an optimization.
    //     ImGui::End();
    //     return;
    // }

    // ImGui::Text(std::format("normal {} {} {}", normal.x, normal.y, normal.z).c_str());
    // ImGui::Text(std::format("radius {}", radius).c_str());
    // ImGui::Text(std::format("center {} {} {}", center.x, center.y, center.z).c_str());
    // ImGui::Text(std::format("pos {} {} {}", pos.x, pos.y, pos.z).c_str());
    // ImGui::Text(std::format("zfar {}", camera.get_far()).c_str());

    // ImGui::End();

    // glm::mat4 light_projection = glm::ortho(-10.0F, 10.0F, -10.F, 10.0F, camera.get_near(), 10.0F);
    // glm::mat4 light_view = glm::lookAt(
    //     glm::vec3(4.0F, 6.0F, -4.0F),
    //     light.direction,
    //     glm::vec3(0.0F, 1.0F, 0.0F));

    // m_light_space_matrix = light_projection * light_view;
}

void DirectionalShadow::shadowmap_draw(Renderer::ShaderProgram& shader, const std::function<void()>& draw_function)
{
    util_assert(initialized == true, "Light::DirectionalShadow has not been initialized");

    glViewport(0, 0, m_shadowmap.get_width(), m_shadowmap.get_height());
    m_shadowmap.bind();

    glClear(GL_DEPTH_BUFFER_BIT);
    shader.bind();
    shader.set_mat4("light_space_matrix", m_light_space_matrix);
    draw_function();

    m_shadowmap.unbind();

    ImGui::Begin("Shadow Map Debug");
    GLuint textureID = m_shadowmap.get_texture().get_id();
    ImGui::Image((void*)(intptr_t)textureID, ImVec2(256, 256), ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();
}

void DirectionalShadow::set_uniforms(Renderer::ShaderProgram& shader, const char* light_name)
{
    util_assert(initialized == true, "Light::DirectionalShadow has not been initialized");

    GLuint texture_unit = Texture::get_texture_unit();
    m_shadowmap.get_texture().bind(texture_unit);
    shader.set_int(std::format("{}.shadow_map", light_name).c_str(), static_cast<int>(texture_unit));
    shader.set_mat4(std::format("{}.light_space_matrix", light_name).c_str(), m_light_space_matrix);
}

} // Renderer::Light::Pbr