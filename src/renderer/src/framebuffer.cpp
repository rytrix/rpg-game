#include "framebuffer.hpp"

namespace Renderer {

Framebuffer::Framebuffer()
{
    glCreateFramebuffers(1, &id);
}

void Framebuffer::bindTexture(GLenum attachment, GLuint texture, GLint level)
{
    glNamedFramebufferTexture(id, attachment, texture, level);
    auto error = glCheckNamedFramebufferStatus(id, GL_FRAMEBUFFER);
    if (error != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error(std::format("Framebuffer error: {}", error));
    }
}

} // namespace Renderer
