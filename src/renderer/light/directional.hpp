#pragma once

#include "../shader.hpp"
#include "../shadowmap.hpp"

namespace Renderer::Light {

struct DirectionalInfo {
    glm::vec3 direction {};
    glm::vec3 ambient {};
    glm::vec3 diffuse {};
    glm::vec3 specular {};

    float near = 1.0F;
    float far = 20.0F;
    bool shadowmap = false;
};

class Directional : public NoCopyNoMove {
public:
    Directional() = default;
    Directional(DirectionalInfo& info);
    ~Directional();

    void init(DirectionalInfo& info);

    void update(DirectionalInfo& info);

    void shadowmap_draw(Renderer::ShaderProgram& shader, const std::function<void()>& draw_function);

    void set_uniforms(Renderer::ShaderProgram& shader, const char* light_name);

    bool has_shadowmap();

private:
    bool initialized = false;

    DirectionalInfo m_info {};

    struct ShadowMap_Internal {
        glm::mat4 m_light_space_matrix {};
        Renderer::ShadowMap m_shadowmap;
    };
    ShadowMap_Internal* m_shadowmap_internal = nullptr;
};

} // namespace Renderer::Light