#pragma once

namespace Renderer {

struct Framebuffer {
    Framebuffer();
    ~Framebuffer() = default;

    void bind() { glBindFramebuffer(GL_FRAMEBUFFER, id); }
    void unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

    void bindTexture(GLenum attachment, GLuint texture, GLint level);
    GLuint getId() { return id; }

private:
    GLuint id;
};

} // namespace Renderer
