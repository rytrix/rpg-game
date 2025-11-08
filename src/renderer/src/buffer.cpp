#include "buffer.hpp"

namespace Renderer {

Buffer::Buffer()
{
    glCreateBuffers(1, &id);
}

Buffer::~Buffer()
{
    glDeleteBuffers(1, &id);
}

void Buffer::bufferData(GLsizeiptr size, const void* data, GLenum usage)
{
    glNamedBufferData(id, size, data, usage);
}

// GL_DYNAMIC_STORAGE_BIT, GL_MAP_READ_BIT GL_MAP_WRITE_BIT, GL_MAP_PERSISTENT_BIT, GL_MAP_COHERENT_BIT, and GL_CLIENT_STORAGE_BIT.
void Buffer::bufferStorage(GLsizeiptr size, const void* data, GLbitfield flags)
{
    glNamedBufferStorage(id, size, data, flags);
}

void Buffer::bufferSubData(GLsizeiptr offset, GLsizeiptr size, const void* data)
{
    glNamedBufferSubData(id, offset, size, data);
}

} // namespace Renderer
