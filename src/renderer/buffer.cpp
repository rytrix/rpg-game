#include "buffer.hpp"

namespace Renderer {

void Buffer::init()
{
    util_assert(initialized == false, "already initialized");

    glCreateBuffers(1, &m_id);
    initialized = true;
}

Buffer::~Buffer()
{
    deinit();
}

void Buffer::deinit()
{
    if (initialized) {
        glDeleteBuffers(1, &m_id);
        initialized = false;
    }
}

void Buffer::buffer_data(GLsizeiptr size, const void* data, GLenum usage)
{
    util_assert(initialized == true, "not initialized");
    glNamedBufferData(m_id, size, data, usage);
}

// GL_DYNAMIC_STORAGE_BIT, GL_MAP_READ_BIT GL_MAP_WRITE_BIT, GL_MAP_PERSISTENT_BIT, GL_MAP_COHERENT_BIT, and GL_CLIENT_STORAGE_BIT.
void Buffer::buffer_storage(GLsizeiptr size, const void* data, GLbitfield flags)
{
    util_assert(initialized == true, "not initialized");
    glNamedBufferStorage(m_id, size, data, flags);
}

void Buffer::buffer_sub_data(GLsizeiptr offset, GLsizeiptr size, const void* data)
{
    util_assert(initialized == true, "not initialized");
    glNamedBufferSubData(m_id, offset, size, data);
}

void Buffer::bind_buffer(GLenum target) const
{
    util_assert(initialized == true, "not initialized");
    glBindBuffer(target, m_id);
}

void Buffer::unbind_buffer(GLenum target) const
{
    util_assert(initialized == true, "not initialized");
    glBindBuffer(target, 0);
}

[[nodiscard]] bool Buffer::is_initialized() const
{
    return initialized;
}

[[nodiscard]] GLuint Buffer::get_id() const
{
    util_assert(initialized == true, "not initialized");
    return m_id;
}

void MappedBuffer::init(u32 buffer_count, u32 buffer_size)
{
    util_assert(initialized == false, "already initialized");
    util_assert(buffer_count <= MAX_FRAME_COUNT, "buffer count greater than max frame count");

    m_current_frame = 0;
    m_frame_count = buffer_count;
    glCreateBuffers(buffer_count, m_ids.data());

    for (u32 i = 0; i < buffer_count; i++) {
        GLuint id = m_ids[i];
        glNamedBufferStorage(id, buffer_size, nullptr, FLAGS);
        m_ptrs[i] = glMapNamedBufferRange(id, 0, buffer_size, FLAGS);
    }

    initialized = true;
}

void MappedBuffer::deinit()
{
    glDeleteBuffers(m_frame_count, m_ids.data());
    m_frame_count = 0;
    m_current_frame = 0;

    initialized = false;
}

MappedBuffer::~MappedBuffer()
{
    deinit();
}

[[nodiscard]] bool MappedBuffer::is_initialized() const
{
    return initialized;
}

[[nodiscard]] GLuint MappedBuffer::get_id() const
{
    util_assert(initialized == true, "not initialized");
    return m_ids[m_current_frame];
}

[[nodiscard]] void* MappedBuffer::get_ptr()
{
    util_assert(initialized == true, "not initialized");
    return m_ptrs[m_current_frame];
}

void MappedBuffer::increment_frame()
{
    util_assert(initialized == true, "not initialized");
    m_current_frame = (m_current_frame + 1) % m_frame_count;
}

} // namespace Renderer
