#pragma once

namespace Renderer {
class Model;
class Mesh;
class Texture;
};

#include "scene/resource_manager.hpp"

#include "renderer/camera.hpp"
#include "renderer/window.hpp"

#include "renderer/default_textures.hpp"

#include "renderer/line_renderer.hpp"
#include "renderer/text.hpp"

#include "scene/entity.hpp"

class GlobalAppData {
public:
    Renderer::Window m_window;
    Renderer::Camera m_camera;

    TextureCache m_texture_cache;
    ModelCache m_model_cache;

    Renderer::DefaultTextures m_default_textures;

    Renderer::TextRenderer text_renderer;
    Renderer::LineRenderer line_renderer;

    Entity selected_entity;
};
