#pragma once

#include "../../shader.hpp"

namespace Renderer::Light::Pbr {

struct Point {
    glm::vec3 position;
    glm::vec3 color;

    void set_uniforms(Renderer::ShaderProgram& shader, const char* light_name);
};

} // Renderer::Light::Pbr