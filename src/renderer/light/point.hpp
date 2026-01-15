#pragma once

#include "../shader.hpp"
#include "../shadowmap.hpp"

namespace Renderer::Light {

class Point {
public:
    Point() = default;
    ~Point();

    Point(const Point&) = delete;
    Point& operator=(const Point&) = delete;
    Point(Point&&) = default;
    Point& operator=(Point&&) = default;

    void init(bool shadowmap, glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, float linear, float quadratic);

    void shadowmap_draw(Renderer::ShaderProgram& shader, glm::mat4& model, const std::function<void()>& draw_function);

    void set_uniforms(Renderer::ShaderProgram& shader, const char* light_name);

    bool has_shadowmap();

    glm::vec3 m_pos {};
    glm::vec3 m_ambient {};
    glm::vec3 m_diffuse {};
    glm::vec3 m_specular {};

    // https://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation
    float m_constant {};
    float m_linear {};
    float m_quadratic {};

private:
    bool initialized = false;

    bool m_shadowmap_enabled {};
    struct ShadowMap_Internal {
        std::array<glm::mat4, 6> m_light_space_matrix {};
        Renderer::ShadowMap m_shadowmap;
        float m_far;
    };
    ShadowMap_Internal* m_shadowmap_internal = nullptr;
};

} // namespace Renderer::Light