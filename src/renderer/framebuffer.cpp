#include "framebuffer.hpp"

namespace Renderer {

Framebuffer::Framebuffer()
    :m_id(0)
{
    glCreateFramebuffers(1, &m_id);
}

void Framebuffer::bind_texture(GLenum attachment, GLuint texture, GLint level) const
{
    glNamedFramebufferTexture(m_id, attachment, texture, level);
    auto error = glCheckNamedFramebufferStatus(m_id, GL_FRAMEBUFFER);
    if (error != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error(std::format("Framebuffer error: {}", error));
    }
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
