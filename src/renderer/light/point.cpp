#include "point.hpp"

namespace Renderer::Light {

Point::~Point()
{
    if (initialized) {
        if (m_shadowmap_enabled) {
            delete m_shadowmap_internal;
        }
        initialized = false;
    }
}

void Point::init(bool shadowmap,
    glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular,
    float constant, float linear, float quadratic)
{
    util_assert(initialized == false, "Point::init() has already been initialized");

    m_pos = position;
    m_ambient = ambient;
    m_diffuse = diffuse;
    m_specular = specular;

    m_constant = constant;
    m_linear = linear;
    m_quadratic = quadratic;

    if (shadowmap) {
        m_shadowmap_enabled = true;

        m_shadowmap_internal = new ShadowMap_Internal();
        m_shadowmap_internal->m_shadowmap.init_cubemap();

        float aspect = m_shadowmap_internal->m_shadowmap.get_width() / m_shadowmap_internal->m_shadowmap.get_height();
        float near = 1.0F;
        float far = 25.0F;

        glm::mat4 light_projection = glm::perspective(glm::radians(90.0F), aspect, near, far);
        m_shadowmap_internal->m_light_space_matrix.at(0) = light_projection * glm::lookAt(m_pos, m_pos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
        m_shadowmap_internal->m_light_space_matrix.at(1) = light_projection * glm::lookAt(m_pos, m_pos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
        m_shadowmap_internal->m_light_space_matrix.at(2) = light_projection * glm::lookAt(m_pos, m_pos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
        m_shadowmap_internal->m_light_space_matrix.at(3) = light_projection * glm::lookAt(m_pos, m_pos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));
        m_shadowmap_internal->m_light_space_matrix.at(4) = light_projection * glm::lookAt(m_pos, m_pos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));
        m_shadowmap_internal->m_light_space_matrix.at(5) = light_projection * glm::lookAt(m_pos, m_pos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));
        m_shadowmap_internal->m_far = far;
    }

    initialized = true;
}

void Point::shadowmap_draw(Renderer::ShaderProgram& shader, glm::mat4& model, const std::function<void()>& draw_function)
{
    util_assert(initialized == true, "Light::Point has not been initialized");
    util_assert(m_shadowmap_enabled == true, "Trying to call shadowmap_draw on a point light without a shadowmap enabled");

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
    shader.set_mat4("model", model);
    shader.set_float("far_plane", m_shadowmap_internal->m_far);
    shader.set_vec3("light_pos", m_pos);
    draw_function();

    m_shadowmap_internal->m_shadowmap.unbind();
}

void Point::set_uniforms(Renderer::ShaderProgram& shader, const char* light_name)
{
    util_assert(initialized == true, "Point has not been initialized");

    shader.set_vec3(std::format("{}.pos", light_name).c_str(), m_pos);
    shader.set_vec3(std::format("{}.ambient", light_name).c_str(), m_ambient);
    shader.set_vec3(std::format("{}.diffuse", light_name).c_str(), m_diffuse);
    shader.set_vec3(std::format("{}.specular", light_name).c_str(), m_specular);
    shader.set_float(std::format("{}.constant", light_name).c_str(), m_constant);
    shader.set_float(std::format("{}.linear", light_name).c_str(), m_linear);
    shader.set_float(std::format("{}.quadratic", light_name).c_str(), m_quadratic);
    if (m_shadowmap_enabled) {
        shader.set_float(std::format("{}.far_plane", light_name).c_str(), m_shadowmap_internal->m_far);
        m_shadowmap_internal->m_shadowmap.get_texture().bind(5);
        shader.set_int(std::format("{}.shadow_map", light_name).c_str(), 5);
    }
}

} // namespace Renderer::Light