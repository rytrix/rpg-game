#pragma once

namespace Renderer {

struct Buffer : public NoCopyNoMove {
    Buffer() = default;
    ~Buffer();

    void init();

    void buffer_data(GLsizeiptr size, const void* data, GLenum usage);
    void buffer_storage(GLsizeiptr size, const void* data, GLbitfield flags);
    void buffer_sub_data(GLsizeiptr offset, GLsizeiptr size, const void* data);

    void bind_buffer(GLenum target) const;
    void unbind_buffer(GLenum target) const;

    [[nodiscard]] GLuint get_id() const;

private:
    bool initialized = false;
    GLuint m_id {};
};

} // namespace Renderer
