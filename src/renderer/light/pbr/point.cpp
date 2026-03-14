#include "point.hpp"

namespace Renderer::Light::Pbr {

void Point::set_uniforms(Renderer::ShaderProgram& shader, const char* light_name) const
{
    shader.set_vec3(std::format("{}.position", light_name).c_str(), position);
    shader.set_vec3(std::format("{}.color", light_name).c_str(), color);
}

void PointShadow::init()
{
    util_assert(initialized == false, "Light::PointShadow has already been initialized");
    m_shadowmap.init_cubemap(2000, 2000);
    initialized = true;
}

PointShadow::~PointShadow()
{
    if (initialized) {
        initialized = false;
    }
}

void PointShadow::update(const Point& light)
{
    util_assert(initialized == true, "Light::PointShadow has not been initialized");

    f32 aspect = static_cast<f32>(m_shadowmap.get_width()) / static_cast<f32>(m_shadowmap.get_height());

    glm::mat4 light_projection = glm::perspective(glm::radians(90.0F), aspect, m_near, m_far);
    m_light_space_matrices.at(0) = light_projection * glm::lookAt(light.position, light.position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
    m_light_space_matrices.at(1) = light_projection * glm::lookAt(light.position, light.position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
    m_light_space_matrices.at(2) = light_projection * glm::lookAt(light.position, light.position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
    m_light_space_matrices.at(3) = light_projection * glm::lookAt(light.position, light.position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));
    m_light_space_matrices.at(4) = light_projection * glm::lookAt(light.position, light.position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));
    m_light_space_matrices.at(5) = light_projection * glm::lookAt(light.position, light.position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));
}

void PointShadow::shadowmap_begin()
{
    util_assert(initialized == true, "Light::PointShadow has not been initialized");

    glViewport(0, 0, m_shadowmap.get_width(), m_shadowmap.get_height());
    m_shadowmap.bind();
    glClear(GL_DEPTH_BUFFER_BIT);
}

void PointShadow::shadowmap_draw(Renderer::ShaderProgram& shader, const Point& light, Renderer::Model* model)
{
    util_assert(initialized == true, "Light::PointShadow has not been initialized");

    shader.bind();

    shader.set_float("far_plane", m_far);
    shader.set_vec3("light_pos", light.position);
    if constexpr (USE_GEOMETRY_SHADER) {
        shader.set_mat4("light_space_matrices[0]", m_light_space_matrices[0]);
        shader.set_mat4("light_space_matrices[1]", m_light_space_matrices[1]);
        shader.set_mat4("light_space_matrices[2]", m_light_space_matrices[2]);
        shader.set_mat4("light_space_matrices[3]", m_light_space_matrices[3]);
        shader.set_mat4("light_space_matrices[4]", m_light_space_matrices[4]);
        shader.set_mat4("light_space_matrices[5]", m_light_space_matrices[5]);
        model->draw_untextured(shader);
    } else {
        for (u32 i = 0; i < 6; i++) {
            m_shadowmap.bind_texture_layer(static_cast<i32>(i));
            shader.set_mat4("light_space_matrix", m_light_space_matrices.at(i));
            model->draw_untextured(shader);
        }
    }
}

void PointShadow::shadowmap_end()
{
    util_assert(initialized == true, "Light::PointShadow has not been initialized");

    m_shadowmap.unbind();
}

void PointShadow::set_uniforms(Renderer::ShaderProgram& shader, const char* light_name)
{
    util_assert(initialized == true, "Light::PointShadow has not been initialized");

    shader.set_float(std::format("{}.far_plane", light_name).c_str(), m_far);
    GLuint texture_unit = Texture::get_texture_unit();
    m_shadowmap.get_texture().bind(texture_unit);
    shader.set_int(std::format("{}.shadow_map", light_name).c_str(), static_cast<int>(texture_unit));
}

} // Renderer::Light::Pbr