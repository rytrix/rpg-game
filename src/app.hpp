#pragma once

#include "renderer.hpp"
#include "scene/scene.hpp"

class App : public NoCopyNoMove {
public:
    App();
    ~App();

    void run();

private:
    void fps_counter();

    bool m_capture_mouse = true;
    bool m_physics_on = false;

    const char* m_window_title = "test window";
    Renderer::Window m_window;
    Renderer::Camera* m_camera;

    Scene* m_scene;
};