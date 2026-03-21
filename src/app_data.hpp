#pragma once

#include "renderer/window.hpp"
#include "renderer/camera.hpp"
#include "scene/resource_manager.hpp"

struct GlobalAppData {
    Renderer::Window m_window;
    Renderer::Camera m_camera;
    SceneResources m_resources;
};
