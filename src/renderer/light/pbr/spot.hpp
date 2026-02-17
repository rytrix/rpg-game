#pragma once

#include "../../shader.hpp"
#include "../../shadowmap.hpp"

namespace Renderer::Light::Pbr {

struct Spot {
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 color;
    f32 inner_cutoff;
    f32 outer_cutoff;

    void set_uniforms(Renderer::ShaderProgram& shader, const char* light_name) const;
};

class SpotShadow : public NoCopyNoMove {
public:
    SpotShadow() = default;
    ~SpotShadow();

    void init();
    void update(const Spot& light);
    void shadowmap_draw(Renderer::ShaderProgram& shader, const std::function<void()>& draw_function);
    void set_uniforms(Renderer::ShaderProgram& shader, const char* light_name);

private:
    bool initialized = false;

    ShadowMap m_shadowmap;
    glm::mat4 m_light_space_matrix {};
    float m_near = 1.0F;
    float m_far = 100.0F;
};

} // Renderer::Light::Pbr