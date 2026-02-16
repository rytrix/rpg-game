#pragma once

#include "../../camera.hpp"
#include "../../shader.hpp"
#include "../../shadowmap.hpp"

namespace Renderer::Light::Pbr {

struct Directional {
    glm::vec3 direction;
    glm::vec3 color;
    
    void set_uniforms(Renderer::ShaderProgram& shader, const char* light_name) const;
};

class DirectionalShadow : public NoCopyNoMove {
public:
    DirectionalShadow() = default;
    ~DirectionalShadow();

    void init();
    void update(const Directional& light, Renderer::Camera& camera);
    void shadowmap_draw(Renderer::ShaderProgram& shader, const std::function<void()>& draw_function);
    void set_uniforms(Renderer::ShaderProgram& shader, const char* light_name);

private:
    bool initialized = false;
    Renderer::ShadowMap m_shadowmap;
    glm::mat4 m_light_space_matrix {};
};

} // Renderer::Light::Pbr