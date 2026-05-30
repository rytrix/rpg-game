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

#include "renderer/debug_lines.hpp"
#include "renderer/text.hpp"

struct GlobalAppData {
    Renderer::Window m_window;
    Renderer::Camera m_camera;

    TextureCache m_texture_cache;
    ModelCache m_model_cache;

    Renderer::DefaultTextures m_default_textures;

    Renderer::TextRenderer text_renderer;
    Renderer::LineRenderer debug_renderer;

    entt::entity selected_entity = entt::null;
};
