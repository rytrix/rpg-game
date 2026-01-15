#include "directional.hpp"

namespace Renderer::Light {

Directional::~Directional()
{
    if (initialized) {
        if (m_shadowmap_enabled) {
            delete m_shadowmap_internal;
        }
        initialized = false;
    }
}

void Directional::init(bool shadowmap, glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular)
{
    util_assert(initialized == false, "Directional::init() has already been initialized");

    m_direction = direction;
    m_ambient = ambient;
    m_diffuse = diffuse;
    m_specular = specular;

    if (shadowmap) {
        m_shadowmap_enabled = true;

        glm::mat4 light_projection = glm::ortho(-10.0F, 10.0F, -10.F, 10.0F, 1.0F, 20.0F);
        glm::mat4 light_view = glm::lookAt(
            glm::vec3(4.0F, 6.0F, -4.0F),
            m_direction,
            glm::vec3(0.0F, 1.0F, 0.0F));

        m_shadowmap_internal = new ShadowMap_Internal();
        m_shadowmap_internal->m_light_space_matrix = light_projection * light_view;
        m_shadowmap_internal->m_shadowmap.init();
    }

    initialized = true;
}

void Directional::shadowmap_draw(Renderer::ShaderProgram& shader, glm::mat4& model, const std::function<void()>& draw_function)
{
    util_assert(initialized == true, "Light::Directional has not been initialized");
    util_assert(m_shadowmap_enabled == true, "Trying to call shadowmap_draw on a directional light without a shadowmap enabled");

    glViewport(0, 0, m_shadowmap_internal->m_shadowmap.get_width(), m_shadowmap_internal->m_shadowmap.get_height());
    m_shadowmap_internal->m_shadowmap.bind();

    glClear(GL_DEPTH_BUFFER_BIT);
    shader.bind();
    shader.set_mat4("light_space_matrix", m_shadowmap_internal->m_light_space_matrix);
    shader.set_mat4("model", model);
    draw_function();

    m_shadowmap_internal->m_shadowmap.unbind();
}

void Directional::set_uniforms(Renderer::ShaderProgram& shader, const char* light_name)
{
    util_assert(initialized == true, "Light::Directional has not been initialized");

    shader.set_vec3(std::format("{}.direction", light_name).c_str(), m_direction);
    shader.set_vec3(std::format("{}.ambient", light_name).c_str(), m_ambient);
    shader.set_vec3(std::format("{}.diffuse", light_name).c_str(), m_diffuse);
    shader.set_vec3(std::format("{}.specular", light_name).c_str(), m_specular);
    if (m_shadowmap_enabled) {
        shader.set_mat4(std::format("{}.light_space_matrix", light_name).c_str(), m_shadowmap_internal->m_light_space_matrix);
        m_shadowmap_internal->m_shadowmap.get_texture().bind(4);
        shader.set_int(std::format("{}.shadow_map", light_name).c_str(), 4);
    }
}

bool Directional::has_shadowmap()
{
    util_assert(initialized == true, "Light::Directional has not been initialized");
    return m_shadowmap_enabled;
}

} // namespace Renderer::Light