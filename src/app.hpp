#pragma once

#include "renderer.hpp"
#include "scene/scene.hpp"

#include "app_data.hpp"

class App : public NoCopyNoMove {
public:
    App();
    ~App();

    void run();

private:
    void fps_counter();

    bool m_capture_mouse = true;
    bool m_physics_on = false;
    bool m_vsync = true;

    const char* m_window_title = "test window";

    GlobalAppData m_data;

    Scene* m_scene = nullptr;
};
