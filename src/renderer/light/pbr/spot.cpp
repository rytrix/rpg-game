#include "spot.hpp"

namespace Renderer::Light::Pbr {

void Spot::set_uniforms(Renderer::ShaderProgram& shader, const char* light_name) const
{
    shader.set_vec3(std::format("{}.position", light_name).c_str(), position);
    shader.set_vec3(std::format("{}.direction", light_name).c_str(), direction);
    shader.set_vec3(std::format("{}.color", light_name).c_str(), color);
    shader.set_float(std::format("{}.inner_cutoff", light_name).c_str(), inner_cutoff);
    shader.set_float(std::format("{}.outer_cutoff", light_name).c_str(), outer_cutoff);
}

} // Renderer::Light::Pbr