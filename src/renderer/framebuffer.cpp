#include "framebuffer.hpp"

namespace Renderer {

Framebuffer::Framebuffer()
    : m_id(0)
{
    glCreateFramebuffers(1, &m_id);
}

GLenum Framebuffer::check_status(GLenum target) const
{
    return glCheckNamedFramebufferStatus(m_id, target);
}

void Framebuffer::bind_renderbuffer(GLenum attachment, GLenum renderbuffer_target, GLuint renderbuffer) const
{
    glNamedFramebufferRenderbuffer(m_id, attachment, renderbuffer_target, renderbuffer);
}

void Framebuffer::bind_texture(GLenum attachment, GLuint texture, GLint level) const
{
    glNamedFramebufferTexture(m_id, attachment, texture, level);
    auto error = glCheckNamedFramebufferStatus(m_id, GL_FRAMEBUFFER);
    if (error != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error(std::format("Framebuffer error: {}", error));
    }
}

void Framebuffer::bind_draw_buffers(GLsizei count, const GLenum* buffs) const
{
    glNamedFramebufferDrawBuffers(m_id, count, buffs);
}

void Framebuffer::bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_id);
}

void Framebuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

[[nodiscard]] GLuint Framebuffer::get_id() const
{
    return m_id;
}

} // namespace Renderer
