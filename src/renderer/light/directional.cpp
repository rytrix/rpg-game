#include "directional.hpp"

namespace Renderer::Light {

Directional::Directional(DirectionalInfo& info)
{
    init(info);
}

Directional::~Directional()
{
    if (initialized) {
        if (m_info.shadowmap) {
            delete m_shadowmap_internal;
        }
        initialized = false;
    }
}

void Directional::init(DirectionalInfo& info)
{
    util_assert(initialized == false, "Directional::init() has already been initialized");

    if (info.shadowmap) {
        m_shadowmap_internal = new ShadowMap_Internal();
        m_shadowmap_internal->m_shadowmap.init();
    }

    m_info = info;
    initialized = true;
    update(info);
}

void Directional::update(DirectionalInfo& info)
{
    util_assert(initialized == true, "Light::Directional has not been initialized");
    util_assert(initialized == true && m_info.shadowmap == info.shadowmap, "Directional::update() cannot change shadowmap value through update function");

    if (info.shadowmap) {
        glm::mat4 light_projection = glm::ortho(-10.0F, 10.0F, -10.F, 10.0F, info.near, info.far);
        glm::mat4 light_view = glm::lookAt(
            glm::vec3(4.0F, 6.0F, -4.0F),
            info.direction,
            glm::vec3(0.0F, 1.0F, 0.0F));

        m_shadowmap_internal->m_light_space_matrix = light_projection * light_view;
    }

    m_info = info;
}

void Directional::shadowmap_draw(Renderer::ShaderProgram& shader, const std::function<void()>& draw_function)
{
    util_assert(initialized == true, "Light::Directional has not been initialized");
    util_assert(m_info.shadowmap == true, "Trying to call shadowmap_draw on a directional light without a shadowmap enabled");

    glViewport(0, 0, m_shadowmap_internal->m_shadowmap.get_width(), m_shadowmap_internal->m_shadowmap.get_height());
    m_shadowmap_internal->m_shadowmap.bind();

    glClear(GL_DEPTH_BUFFER_BIT);
    shader.bind();
    shader.set_mat4("light_space_matrix", m_shadowmap_internal->m_light_space_matrix);
    draw_function();

    m_shadowmap_internal->m_shadowmap.unbind();
}

void Directional::set_uniforms(Renderer::ShaderProgram& shader, const char* light_name)
{
    util_assert(initialized == true, "Light::Directional has not been initialized");

    shader.set_vec3(std::format("{}.direction", light_name).c_str(), m_info.direction);
    shader.set_vec3(std::format("{}.ambient", light_name).c_str(), m_info.ambient);
    shader.set_vec3(std::format("{}.diffuse", light_name).c_str(), m_info.diffuse);
    shader.set_vec3(std::format("{}.specular", light_name).c_str(), m_info.specular);
    if (m_info.shadowmap) {
        shader.set_mat4(std::format("{}.light_space_matrix", light_name).c_str(), m_shadowmap_internal->m_light_space_matrix);
        GLuint texture_unit = Texture::get_texture_unit();
        m_shadowmap_internal->m_shadowmap.get_texture().bind(texture_unit);
        shader.set_int(std::format("{}.shadow_map", light_name).c_str(), static_cast<int>(texture_unit));
    }
}

bool Directional::has_shadowmap()
{
    util_assert(initialized == true, "Light::Directional has not been initialized");
    return m_info.shadowmap;
}

} // namespace Renderer::Light