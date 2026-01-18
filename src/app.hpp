#pragma once

#include "utils/deltatime.hpp"

#include "physics/engine.hpp"

#include "renderer.hpp"

class App {
public:
    App();
    ~App() = default;

    App(const App&) = delete;
    App& operator=(const App&) = delete;
    App(App&&) = default;
    App& operator=(App&&) = default;

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

    std::unique_ptr<PhysicsEngine> m_physics_engine = nullptr;

    Renderer::Window m_window;
    Renderer::Camera m_camera;

    Renderer::GBuffer m_gpass;
    Renderer::Quad m_lpass;

    Renderer::ShaderProgram m_gpass_shader;
    Renderer::ShaderProgram m_lpass_shader;
    Renderer::ShaderProgram m_shadowmap_shader;
    Renderer::ShaderProgram m_shadowmap_cubemap_shader;

    Renderer::Model m_plane;

    Renderer::Model m_cube;
    JPH::BodyID b_cube;

    Renderer::Light::Directional m_directional_light;
    Renderer::Light::Point m_point_light;
};