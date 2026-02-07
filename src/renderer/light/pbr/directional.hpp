#pragma once

#include "../../shader.hpp"

namespace Renderer::Light::Pbr {

struct Directional {
    glm::vec3 direction;
    glm::vec3 color;

    void set_uniforms(Renderer::ShaderProgram& shader, const char* light_name);
};

} // Renderer::Light::Pbr