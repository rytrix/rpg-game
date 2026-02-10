#include "directional.hpp"

namespace Renderer::Light::Pbr {

void Directional::set_uniforms(Renderer::ShaderProgram& shader, const char* light_name) const
{
    shader.set_vec3(std::format("{}.direction", light_name).c_str(), direction);
    shader.set_vec3(std::format("{}.color", light_name).c_str(), color);
}

} // Renderer::Light::Pbr