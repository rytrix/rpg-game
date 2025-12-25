#pragma once

#include "../shader.hpp"
#include "../shadowmap.hpp"

namespace Renderer::Light {

class Directional {
public:
    Directional() = default;
    ~Directional();

    Directional(const Directional&) = delete;
    Directional& operator=(const Directional&) = delete;
    Directional(Directional&&) = default;
    Directional& operator=(Directional&&) = default;

    void init(bool shadowmap, glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular);

    void shadowmap_draw(Renderer::ShaderProgram& shader, glm::mat4& model, const std::function<void()>& draw_function);

    void set_uniforms(Renderer::ShaderProgram& shader, const char* light_name);

    glm::vec3 m_direction {};
    glm::vec3 m_ambient {};
    glm::vec3 m_diffuse {};
    glm::vec3 m_specular {};

private:
    bool initialized = false;

    bool m_shadowmap_enabled {};
    struct ShadowMap_Internal {
        glm::mat4 m_light_space_matrix {};
        Renderer::ShadowMap m_shadowmap;
    };
    ShadowMap_Internal* m_shadowmap_internal = nullptr;
};

} // namespace Renderer::Light