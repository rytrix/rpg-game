#include "buffer.hpp"

namespace Renderer {

void Buffer::init()
{
    if (initialized) {
        throw std::runtime_error("Buffer::init() Attempting to reinitialize opengl buffer");
    }
    glCreateBuffers(1, &m_id);
    initialized = true;
}

Buffer::~Buffer()
{
    if (initialized) {
        glDeleteBuffers(1, &m_id);
        initialized = false;
    }
}

void Buffer::buffer_data(GLsizeiptr size, const void* data, GLenum usage)
{
    glNamedBufferData(m_id, size, data, usage);
}

// GL_DYNAMIC_STORAGE_BIT, GL_MAP_READ_BIT GL_MAP_WRITE_BIT, GL_MAP_PERSISTENT_BIT, GL_MAP_COHERENT_BIT, and GL_CLIENT_STORAGE_BIT.
void Buffer::buffer_storage(GLsizeiptr size, const void* data, GLbitfield flags)
{
    glNamedBufferStorage(m_id, size, data, flags);
}

void Buffer::buffer_sub_data(GLsizeiptr offset, GLsizeiptr size, const void* data)
{
    glNamedBufferSubData(m_id, offset, size, data);
}

[[nodiscard]] GLuint Buffer::get_id() const
{
    return m_id;
}

} // namespace Renderer
