#pragma once

#include "../../shader.hpp"

namespace Renderer::Light::Pbr {

struct Spot {
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 color;
    f32 inner_cutoff;
    f32 outer_cutoff;

    void set_uniforms(Renderer::ShaderProgram& shader, const char* light_name) const;
};

} // Renderer::Light::Pbr