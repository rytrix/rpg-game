#pragma once

#include "../../shader.hpp"
#include "../../shadowmap.hpp"

namespace Renderer::Light::Pbr {

struct Directional {
    glm::vec3 direction;
    glm::vec3 color;

    // glm::mat4 m_light_space_matrix {};
    // Renderer::ShadowMap m_shadowmap;

    void set_uniforms(Renderer::ShaderProgram& shader, const char* light_name) const;
    void init_shadowmap();
    void update();
    void shadowmap_draw(Renderer::ShaderProgram& shader, const std::function<void()>& draw_function);
};

} // Renderer::Light::Pbr