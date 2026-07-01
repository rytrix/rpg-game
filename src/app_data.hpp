#pragma once

#include "scene/resource_manager.hpp"

#include "renderer/camera.hpp"
#include "renderer/window.hpp"

#include "renderer/default_textures.hpp"

#include "renderer/line_renderer.hpp"
#include "renderer/text.hpp"

#include "scene/components/entity_selector.hpp"
#include "scene/components/gizmo.hpp"

struct GlobalAppData {
    Renderer::Window m_window;
    Renderer::Camera m_camera;

    TextureCache m_texture_cache;
    ModelCache m_model_cache;

    Renderer::DefaultTextures m_default_textures;

    Renderer::TextRenderer m_text_renderer;
    Renderer::LineRenderer m_line_renderer;

    EntitySelector m_entity_selector;
    Gizmo m_gizmo;

    bool m_capture_mouse = true;
};
