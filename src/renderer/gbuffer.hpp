#pragma once

#include "framebuffer.hpp"
#include "renderbuffer.hpp"
#include "shader.hpp"
#include "texture.hpp"

namespace Renderer {

class GBuffer : public NoCopyNoMove {
public:
    GBuffer() = default;
    ~GBuffer();

    void init(int screen_width, int screen_height);
    void reinit(int screen_width, int screen_height);
    void bind();
    void unbind();

    void blit_depth_buffer();

    void set_uniforms(Renderer::ShaderProgram& shader);

private:
    bool initialized = false;

    Renderer::Framebuffer m_buffer;

    Renderer::Texture m_position;
    Renderer::Texture m_normal;
    Renderer::Texture m_albedo;

    Renderer::Renderbuffer m_depth;

    i32 m_buffer_width {};
    i32 m_buffer_height {};
};

} // namespace Renderer
