#pragma once

namespace Renderer {

class Framebuffer {
public:
    Framebuffer();
    ~Framebuffer() = default;

    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;
    Framebuffer(Framebuffer&&) = default;
    Framebuffer& operator=(Framebuffer&&) = default;

    void bind() const;
    static void unbind();

    void bind_texture(GLenum attachment, GLuint texture, GLint level) const;
    [[nodiscard]] GLuint get_id() const;

private:
    GLuint m_id;
};

} // namespace Renderer
