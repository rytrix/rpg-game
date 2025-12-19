#include "gbuffer.hpp"

namespace Renderer {

void GBuffer::init(int screen_width, int screen_height)
{
    m_buffer.init();

    Renderer::TextureInfo texture_info;
    texture_info.size = Renderer::TextureSize { .width = screen_width, .height = screen_height, .depth = 0 };
    texture_info.internal_format = GL_RGBA16F;
    texture_info.mipmaps = GL_FALSE;
    m_position.init(texture_info);
    m_buffer.bind_texture(GL_COLOR_ATTACHMENT0, m_position.get_id(), 0);

    m_normal.init(texture_info);
    m_buffer.bind_texture(GL_COLOR_ATTACHMENT1, m_normal.get_id(), 0);

    texture_info.internal_format = GL_RGBA16F;
    m_albedo.init(texture_info);
    m_buffer.bind_texture(GL_COLOR_ATTACHMENT2, m_albedo.get_id(), 0);

    m_depth.init();
    m_depth.buffer_storage(GL_DEPTH_COMPONENT24, screen_width, screen_height);

    m_buffer.bind_renderbuffer(GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth.get_id());

    std::array<u32, 3> attachments = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    m_buffer.bind_draw_buffers(attachments.size(), attachments.data());
}

void GBuffer::reinit(int screen_width, int screen_height)
{
    this->~GBuffer();
    init(screen_width, screen_height);
}

void GBuffer::bind()
{
    m_buffer.bind();
}

void GBuffer::set_uniforms(Renderer::ShaderProgram& shader)
{
    m_position.bind(0);
    shader.set_int("gPosition", 0);

    m_normal.bind(1);
    shader.set_int("gNormal", 1);

    m_albedo.bind(2);
    shader.set_int("gAlbedoSpec", 2);
}

void GBuffer::unbind()
{
    m_buffer.unbind();
}

} // namespace Renderer
