#pragma once

namespace Renderer {

class Framebuffer : public NoCopyNoMove {
public:
    Framebuffer() = default;
    ~Framebuffer();

    void init();

    void bind() const;
    static void unbind();

    [[nodiscard]] GLenum check_status(GLenum target) const;

    void bind_texture(GLenum attachment, GLuint texture, GLint level) const;
    void bind_renderbuffer(GLenum attachment, GLenum renderbuffer_target, GLuint renderbuffer) const;
    void bind_draw_buffer(const GLenum buff) const;
    void bind_draw_buffers(GLsizei count, const GLenum* buffs) const;
    void bind_read_buffer(const GLenum buff) const;

    [[nodiscard]] GLuint get_id() const;

private:
    bool initialized = false;
    GLuint m_id {};
};

} // namespace Renderer
