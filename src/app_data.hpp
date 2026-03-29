#pragma once

#include "renderer/window.hpp"
#include "renderer/camera.hpp"
#include "renderer/default_textures.hpp"
#include "scene/resource_manager.hpp"

struct GlobalAppData {
    Renderer::Window m_window;
    Renderer::Camera m_camera;

    TextureCache m_texture_cache;
    ModelCache m_model_cache;
    
    Renderer::DefaultTextures m_default_textures;
};
