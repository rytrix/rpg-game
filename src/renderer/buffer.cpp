#include "buffer.hpp"

namespace Renderer {

void Buffer::init()
{
    util_assert(initialized == false, "Buffer::init() has already been initialized");

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
    util_assert(initialized == true, "Buffer has not been initialized");
    glNamedBufferData(m_id, size, data, usage);
}

// GL_DYNAMIC_STORAGE_BIT, GL_MAP_READ_BIT GL_MAP_WRITE_BIT, GL_MAP_PERSISTENT_BIT, GL_MAP_COHERENT_BIT, and GL_CLIENT_STORAGE_BIT.
void Buffer::buffer_storage(GLsizeiptr size, const void* data, GLbitfield flags)
{
    util_assert(initialized == true, "Buffer has not been initialized");
    glNamedBufferStorage(m_id, size, data, flags);
}

void Buffer::buffer_sub_data(GLsizeiptr offset, GLsizeiptr size, const void* data)
{
    util_assert(initialized == true, "Buffer has not been initialized");
    glNamedBufferSubData(m_id, offset, size, data);
}

[[nodiscard]] GLuint Buffer::get_id() const
{
    util_assert(initialized == true, "Buffer has not been initialized");
    return m_id;
}

} // namespace Renderer
