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
    void update(const Directional& light, const Renderer::Camera& camera);
    void shadowmap_draw(const std::function<void(Renderer::ShaderProgram& shader)>& draw_function);
    void set_uniforms(Renderer::ShaderProgram& shader, const char* light_name);

private:
    glm::mat4 calculate_light_space_matrix(const Directional& light, const glm::mat4 proj, const glm::mat4 view, f32 far);

    static std::array<std::string, 2> get_directional_cascade_shader_text();
    static std::array<std::string, 3> get_directional_cascade_shader_text_geometry(u32 cascade_count);

    static constexpr bool USE_GEOMETRY_SHADER = false;
    static constexpr usize MAX_CASCADES = 16;
    bool initialized = false;
    u32 m_cascades = 6;
    Renderer::ShaderProgram m_shader;
    Renderer::ShadowMap m_shadowmap;
    std::array<glm::mat4, MAX_CASCADES> m_light_space_matrix;
    std::array<f32, MAX_CASCADES + 1> m_cascade_plane_distances;
};

} // namespace Renderer::Light::Pbr