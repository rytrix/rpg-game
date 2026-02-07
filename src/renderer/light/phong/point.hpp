#pragma once

#include "../shader.hpp"
#include "../shadowmap.hpp"

namespace Renderer::Light::Phong {

struct PointInfo {
    glm::vec3 position {};
    glm::vec3 ambient {};
    glm::vec3 diffuse {};
    glm::vec3 specular {};

    // https://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation
    float constant {};
    float linear {};
    float quadratic {};

    bool shadowmap = false;
    float near = 1.0F;
    float far = 25.0F;
};

class Point : public NoCopyNoMove {
public:
    Point() = default;
    Point(PointInfo& info);
    ~Point();

    void init(PointInfo& info);

    void update(PointInfo& info);

    void shadowmap_draw(Renderer::ShaderProgram& shader, const std::function<void()>& draw_function);

    void set_uniforms(Renderer::ShaderProgram& shader, const char* light_name);

    bool has_shadowmap();

private:
    bool initialized = false;

    PointInfo m_info {};
    struct ShadowMap_Internal {
        std::array<glm::mat4, 6> m_light_space_matrix {};
        Renderer::ShadowMap m_shadowmap;
    };
    ShadowMap_Internal* m_shadowmap_internal = nullptr;
};

} // namespace Renderer::Light::Phong