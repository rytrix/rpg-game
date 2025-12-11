#pragma once

namespace Renderer {

class Framebuffer {
public:
    Framebuffer();
    ~Framebuffer();

    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;
    Framebuffer(Framebuffer&&) = default;
    Framebuffer& operator=(Framebuffer&&) = default;

    void init();

    void bind() const;
    static void unbind();

    [[nodiscard]] GLenum check_status(GLenum target) const;

    void bind_texture(GLenum attachment, GLuint texture, GLint level) const;
    void bind_renderbuffer(GLenum attachment, GLenum renderbuffer_target, GLuint renderbuffer) const;
    void bind_draw_buffers(GLsizei count, const GLenum* buffs) const;

    [[nodiscard]] GLuint get_id() const;

private:
    GLuint m_id {};
};

} // namespace Renderer
