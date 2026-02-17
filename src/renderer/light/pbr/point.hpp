#pragma once

#include "../../camera.hpp"
#include "../../shader.hpp"
#include "../../shadowmap.hpp"

namespace Renderer::Light::Pbr {

struct Point {
    void set_uniforms(Renderer::ShaderProgram& shader, const char* light_name) const;

    glm::vec3 position;
    glm::vec3 color;
};

class PointShadow : public NoCopyNoMove {
public:
    PointShadow() = default;
    ~PointShadow();

    void init();
    void update(const Point& light);
    void shadowmap_draw(Renderer::ShaderProgram& shader, const Point& light, const std::function<void()>& draw_function);
    void set_uniforms(Renderer::ShaderProgram& shader, const char* light_name);

private:
    bool initialized = false;

    ShadowMap m_shadowmap;
    std::array<glm::mat4, 6> m_light_space_matrices {};
    float m_near = 1.0F;
    float m_far = 100.0F;
};

} // Renderer::Light::Pbr