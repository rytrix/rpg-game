#include "framebuffer.hpp"

namespace Renderer {

void Framebuffer::init()
{
    util_assert(initialized == false, "Framebuffer::init() has already been initialized");
    glCreateFramebuffers(1, &m_id);
    initialized = true;
}

Framebuffer::~Framebuffer()
{
    if (initialized) {
        glDeleteFramebuffers(1, &m_id);
        initialized = false;
    }
}

[[nodiscard]] GLenum Framebuffer::check_status(GLenum target) const
{
    util_assert(initialized == true, "Framebuffer has not been initialized");
    return glCheckNamedFramebufferStatus(m_id, target);
}

void Framebuffer::bind_renderbuffer(GLenum attachment, GLenum renderbuffer_target, GLuint renderbuffer) const
{
    util_assert(initialized == true, "Framebuffer has not been initialized");
    glNamedFramebufferRenderbuffer(m_id, attachment, renderbuffer_target, renderbuffer);
}

void Framebuffer::bind_texture(GLenum attachment, GLuint texture, GLint level) const
{
    util_assert(initialized == true, "Framebuffer has not been initialized");
    glNamedFramebufferTexture(m_id, attachment, texture, level);
    auto error = glCheckNamedFramebufferStatus(m_id, GL_FRAMEBUFFER);
    if (error != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error(std::format("Framebuffer error: {}", error));
    }
}

void Framebuffer::bind_draw_buffer(const GLenum buff) const
{
    util_assert(initialized == true, "Framebuffer has not been initialized");
    glNamedFramebufferDrawBuffer(m_id, buff);
}

void Framebuffer::bind_draw_buffers(GLsizei count, const GLenum* buffs) const
{
    util_assert(initialized == true, "Framebuffer has not been initialized");
    glNamedFramebufferDrawBuffers(m_id, count, buffs);
}

void Framebuffer::bind_read_buffer(const GLenum buff) const
{
    util_assert(initialized == true, "Framebuffer has not been initialized");
    glNamedFramebufferReadBuffer(m_id, buff);
}

void Framebuffer::bind() const
{
    util_assert(initialized == true, "Framebuffer has not been initialized");
    glBindFramebuffer(GL_FRAMEBUFFER, m_id);
}

void Framebuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

[[nodiscard]] GLuint Framebuffer::get_id() const
{
    util_assert(initialized == true, "Framebuffer has not been initialized");
    return m_id;
}

} // namespace Renderer
