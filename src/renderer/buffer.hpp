#pragma once

namespace Renderer {

struct Buffer {
    Buffer() = default;
    ~Buffer();

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;
    Buffer(Buffer&&) = default;
    Buffer& operator=(Buffer&&) = default;

    void init();

    void buffer_data(GLsizeiptr size, const void* data, GLenum usage);
    void buffer_storage(GLsizeiptr size, const void* data, GLbitfield flags);
    void buffer_sub_data(GLsizeiptr offset, GLsizeiptr size, const void* data);

    [[nodiscard]] GLuint get_id() const;

private:
    bool initialized = false;
    GLuint m_id {};
};

} // namespace Renderer
