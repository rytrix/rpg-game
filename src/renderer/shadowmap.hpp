#pragma once

#include "framebuffer.hpp"
#include "shader.hpp"
#include "texture.hpp"

namespace Renderer {

class ShadowMap : public NoCopyNoMove {
public:
    ShadowMap() = default;
    ~ShadowMap();

    void init();
    void init(i32 width, i32 height);
    void init_cascade(i32 cascades);
    void init_cascade(i32 width, i32 height, i32 cascades);
    void init_cubemap();
    void init_cubemap(i32 width, i32 height);

    void bind();
    void bind_texture_layer(GLint layer);
    void unbind();

    [[nodiscard]] i32 get_width() const;
    [[nodiscard]] i32 get_height() const;
    [[nodiscard]] Texture& get_texture();
    [[nodiscard]] bool is_initialized() const;

private:
    enum struct MapType {
        Default,
        Cascade,
        CubeMap,
    };

    static constexpr i32 DEFAULT_SHADOW_WIDTH = 2000;
    static constexpr i32 DEFAULT_SHADOW_HEIGHT = 2000;
    static constexpr i32 DEFAULT_CASCADES = 0;

    bool initialized = false;

    i32 m_shadow_width = DEFAULT_SHADOW_WIDTH;
    i32 m_shadow_height = DEFAULT_SHADOW_HEIGHT;
    i32 m_cascades = DEFAULT_CASCADES;
    Renderer::Texture m_texture;
    Renderer::Framebuffer m_framebuffer;

    void init_internal(MapType cubemap);
};

} // namespace Renderer
