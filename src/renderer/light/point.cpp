#include "point.hpp"

namespace Renderer::Light {

Point::Point(PointInfo& info)
{
    init(info);
}

Point::~Point()
{
    if (initialized) {
        if (m_info.shadowmap) {
            delete m_shadowmap_internal;
        }
        initialized = false;
    }
}

void Point::init(PointInfo& info)
{
    util_assert(initialized == false, "Point::init() has already been initialized");

    if (info.shadowmap) {
        m_shadowmap_internal = new ShadowMap_Internal();
        m_shadowmap_internal->m_shadowmap.init_cubemap();
    }

    initialized = true;
    m_info = info;
    update(info);
}

void Point::update(PointInfo& info)
{
    util_assert(initialized == true, "Light::Point has not been initialized");
    util_assert(initialized == true && m_info.shadowmap == info.shadowmap, "Point::update() cannot change shadowmap value through update function");

    if (info.shadowmap) {
        f32 aspect = static_cast<f32>(m_shadowmap_internal->m_shadowmap.get_width()) / static_cast<f32>(m_shadowmap_internal->m_shadowmap.get_height());

        glm::mat4 light_projection = glm::perspective(glm::radians(90.0F), aspect, info.near, info.far);
        m_shadowmap_internal->m_light_space_matrix.at(0) = light_projection * glm::lookAt(info.position, info.position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
        m_shadowmap_internal->m_light_space_matrix.at(1) = light_projection * glm::lookAt(info.position, info.position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
        m_shadowmap_internal->m_light_space_matrix.at(2) = light_projection * glm::lookAt(info.position, info.position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
        m_shadowmap_internal->m_light_space_matrix.at(3) = light_projection * glm::lookAt(info.position, info.position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));
        m_shadowmap_internal->m_light_space_matrix.at(4) = light_projection * glm::lookAt(info.position, info.position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));
        m_shadowmap_internal->m_light_space_matrix.at(5) = light_projection * glm::lookAt(info.position, info.position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));
    }

    m_info = info;
}

void Point::shadowmap_draw(Renderer::ShaderProgram& shader, const std::function<void()>& draw_function)
{
    util_assert(initialized == true, "Light::Point has not been initialized");
    util_assert(m_info.shadowmap == true, "Trying to call shadowmap_draw on a point light without a shadowmap enabled");

    glViewport(0, 0, m_shadowmap_internal->m_shadowmap.get_width(), m_shadowmap_internal->m_shadowmap.get_height());
    m_shadowmap_internal->m_shadowmap.bind();

    glClear(GL_DEPTH_BUFFER_BIT);
    shader.bind();
    shader.set_mat4("light_space_matrices[0]", m_shadowmap_internal->m_light_space_matrix[0]);
    shader.set_mat4("light_space_matrices[1]", m_shadowmap_internal->m_light_space_matrix[1]);
    shader.set_mat4("light_space_matrices[2]", m_shadowmap_internal->m_light_space_matrix[2]);
    shader.set_mat4("light_space_matrices[3]", m_shadowmap_internal->m_light_space_matrix[3]);
    shader.set_mat4("light_space_matrices[4]", m_shadowmap_internal->m_light_space_matrix[4]);
    shader.set_mat4("light_space_matrices[5]", m_shadowmap_internal->m_light_space_matrix[5]);
    shader.set_float("far_plane", m_info.far);
    shader.set_vec3("light_pos", m_info.position);
    draw_function();

    m_shadowmap_internal->m_shadowmap.unbind();
}

void Point::set_uniforms(Renderer::ShaderProgram& shader, const char* light_name)
{
    util_assert(initialized == true, "Point has not been initialized");

    shader.set_vec3(std::format("{}.pos", light_name).c_str(), m_info.position);
    shader.set_vec3(std::format("{}.ambient", light_name).c_str(), m_info.ambient);
    shader.set_vec3(std::format("{}.diffuse", light_name).c_str(), m_info.diffuse);
    shader.set_vec3(std::format("{}.specular", light_name).c_str(), m_info.specular);
    shader.set_float(std::format("{}.constant", light_name).c_str(), m_info.constant);
    shader.set_float(std::format("{}.linear", light_name).c_str(), m_info.linear);
    shader.set_float(std::format("{}.quadratic", light_name).c_str(), m_info.quadratic);
    if (m_info.shadowmap) {
        shader.set_float(std::format("{}.far_plane", light_name).c_str(), m_info.far);

        GLuint texture_unit = Texture::get_texture_unit();
        m_shadowmap_internal->m_shadowmap.get_texture().bind(texture_unit);
        shader.set_int(std::format("{}.shadow_map", light_name).c_str(), static_cast<int>(texture_unit));
    }
}

bool Point::has_shadowmap()
{
    util_assert(initialized == true, "Point has not been initialized");
    return m_info.shadowmap;
}

} // namespace Renderer::Light