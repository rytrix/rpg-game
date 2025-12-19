#pragma once

#include "framebuffer.hpp"
#include "texture.hpp"
#include "renderbuffer.hpp"
#include "shader.hpp"

namespace Renderer {

class GBuffer {
public:
    GBuffer() = default;
    ~GBuffer() = default;

    GBuffer(const GBuffer&) = delete;
    GBuffer& operator=(const GBuffer&) = delete;
    GBuffer(GBuffer&&) = default;
    GBuffer& operator=(GBuffer&&) = default;

    void init(int screen_width, int screen_height);
    void reinit(int screen_width, int screen_height);
    void bind();
    void unbind();

    void set_uniforms(Renderer::ShaderProgram& shader);

private:
    Renderer::Framebuffer m_buffer;

    Renderer::Texture m_position;
    Renderer::Texture m_normal;
    Renderer::Texture m_albedo;

    Renderer::Renderbuffer m_depth;
};

} // namespace Renderer
