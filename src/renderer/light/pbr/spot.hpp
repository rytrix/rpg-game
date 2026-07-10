#pragma once

#include "../../model.hpp"
#include "../../shader.hpp"
#include "../../shadowmap.hpp"

namespace Renderer::Light::Pbr {

struct Spot {
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 color;
    f32 inner_cutoff;
    f32 outer_cutoff;

    f32 inner_cutoff_degrees;
    f32 outer_cutoff_degrees;

    void calculate_cutoffs();
    void set_uniforms(Renderer::Shader& shader, const char* light_name) const;
};

class SpotShadow : public NoCopyNoMove {
public:
    SpotShadow() = default;
    ~SpotShadow();

    void init();
    void update(const Spot& light);
    void shadowmap_begin();
    void shadowmap_draw(Renderer::Shader& shader, Renderer::Mesh* mesh);
    void shadowmap_end();
    void set_uniforms(Renderer::Shader& shader, const char* light_name);

private:
    bool initialized = false;

    ShadowMap m_shadowmap;
    glm::mat4 m_light_space_matrix {};
    float m_near = 1.0F;
    float m_far = 100.0F;
};

} // Renderer::Light::Pbr
