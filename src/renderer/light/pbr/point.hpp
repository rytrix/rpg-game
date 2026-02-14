#pragma once

#include "../../shader.hpp"
#include "../../shadowmap.hpp"

namespace Renderer::Light::Pbr {

struct Point {
    void set_uniforms(Renderer::ShaderProgram& shader, const char* light_name) const;
    void init_shadowmap();
    void update();
    void shadowmap_draw();

    glm::vec3 position;
    glm::vec3 color;

    // ShadowMap shadowmap;
    // std::array<glm::mat4, 6> light_space_matrix {};
    // float near = 1.0F;
    // float far = 25.0F;
};

} // Renderer::Light::Pbr