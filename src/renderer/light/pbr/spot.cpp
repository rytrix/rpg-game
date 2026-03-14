#include "spot.hpp"

namespace Renderer::Light::Pbr {

void Spot::set_uniforms(Renderer::ShaderProgram& shader, const char* light_name) const
{
    shader.set_vec3(std::format("{}.position", light_name).c_str(), position);
    shader.set_vec3(std::format("{}.direction", light_name).c_str(), direction);
    shader.set_vec3(std::format("{}.color", light_name).c_str(), color);
    shader.set_float(std::format("{}.inner_cutoff", light_name).c_str(), inner_cutoff);
    shader.set_float(std::format("{}.outer_cutoff", light_name).c_str(), outer_cutoff);
}

void SpotShadow::init()
{
    util_assert(initialized == false, "Light::SpotShadow has already been initialized");
    m_shadowmap.init();
    initialized = true;
}

SpotShadow::~SpotShadow()
{
    if (initialized) {
        initialized = false;
    }
}

void SpotShadow::update(const Spot& light)
{
    util_assert(initialized == true, "Light::SpotShadow has not been initialized");

    glm::vec3 up = (std::abs(light.direction.y) > 0.99f) ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);
    glm::mat4 light_view = glm::lookAt(
        light.position,
        light.position + light.direction,
        up);

    float fov = glm::degrees(std::acos(light.outer_cutoff)) * 2.0f;
    float aspect = (f32)m_shadowmap.get_width() / (f32)m_shadowmap.get_height();

    glm::mat4 light_proj = glm::perspective(glm::radians(fov), aspect, m_near, m_far);

    m_light_space_matrix = light_proj * light_view;
}

void SpotShadow::shadowmap_begin()
{
    util_assert(initialized == true, "Light::SpotShadow has not been initialized");

    glViewport(0, 0, m_shadowmap.get_width(), m_shadowmap.get_height());
    m_shadowmap.bind();

    glClear(GL_DEPTH_BUFFER_BIT);
}

void SpotShadow::shadowmap_draw(Renderer::ShaderProgram& shader, Renderer::Model* model)
{
    util_assert(initialized == true, "Light::SpotShadow has not been initialized");

    shader.bind();
    shader.set_mat4("light_space_matrix", m_light_space_matrix);
    model->draw_untextured(shader);
}

void SpotShadow::shadowmap_end()
{
    util_assert(initialized == true, "Light::SpotShadow has not been initialized");

    m_shadowmap.unbind();
}

void SpotShadow::set_uniforms(Renderer::ShaderProgram& shader, const char* light_name)
{
    util_assert(initialized == true, "Light::SpotShadow has not been initialized");

    shader.set_mat4(std::format("{}.light_space_matrix", light_name).c_str(), m_light_space_matrix);
    GLuint texture_unit = Texture::get_texture_unit();
    m_shadowmap.get_texture().bind(texture_unit);
    shader.set_int(std::format("{}.shadow_map", light_name).c_str(), static_cast<int>(texture_unit));
}

} // Renderer::Light::Pbr