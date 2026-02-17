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
    glm::mat4 calculate_light_space_matrix(const Directional& light, const glm::mat4 proj, const glm::mat4 view, f32 far);

    bool initialized = false;
    u32 m_cascades = 4;
    Renderer::ShadowMap m_shadowmap;
    std::vector<glm::mat4> m_light_space_matrix;
    std::vector<f32> m_cascade_plane_distances;
};

} // Renderer::Light::Pbr