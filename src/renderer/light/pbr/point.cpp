#include "point.hpp"

namespace Renderer::Light::Pbr {

void Point::set_uniforms(Renderer::ShaderProgram& shader, const char* light_name) const
{
    shader.set_vec3(std::format("{}.position", light_name).c_str(), position);
    shader.set_vec3(std::format("{}.color", light_name).c_str(), color);
}

} // Renderer::Light::Pbr