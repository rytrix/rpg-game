#pragma once

#include "utils/deltatime.hpp"

#include "physics/engine.hpp"

#include "renderer.hpp"

class App : public NoCopyNoMove {
public:
    App();
    ~App();

    void run();

private:
    void frame_counter();
    void keyboard_callback();
    void keyboard_input();
    void imgui_run();

    Utils::DeltaTime m_clock;

    bool m_capture_mouse = true;
    bool m_physics_on = false;

    const char* m_title = "Test Window";

    std::unique_ptr<Physics::System> m_physics_system = nullptr;

    Renderer::Window m_window;
    Renderer::Camera m_camera;

    Renderer::GBuffer m_gpass;
    Renderer::Quad m_lpass;

    Renderer::ShaderProgram m_gpass_shader;
    Renderer::ShaderProgram m_lpass_shader;
    Renderer::ShaderProgram m_shadowmap_shader;
    Renderer::ShaderProgram m_shadowmap_cubemap_shader;

    Renderer::Model m_plane;
    glm::mat4 m_plane_matrix { 1.0 };

    Renderer::Model m_cube;
    glm::mat4 m_cube_matrix { 1.0 };
    JPH::BodyID b_cube;

    Renderer::Light::Directional m_directional_light;
    Renderer::Light::Point m_point_light;
};
