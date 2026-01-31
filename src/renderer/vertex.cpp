#include "vertex.hpp"

namespace Renderer {

void VertexArray::init()
{
    util_assert(initialized == false, "VertexArray::init() has already been initialized");

    glCreateVertexArrays(1, &m_id);
    glBindVertexArray(m_id);

    initialized = true;
}

VertexArray::~VertexArray()
{
    // util_assert(initialized == true, "VertexArray::~VertexArray() has not been initialized");
    if (initialized) {
        glDeleteVertexArrays(1, &m_id);
        initialized = false;
    }
}

void VertexArray::vertex_attrib(GLuint attrib_index, GLuint binding_index, GLint values_per_vertex, GLenum data_type, GLuint relative_offset_in_bytes)
{
    util_assert(initialized == true, "VertexArray has not been initialized");
    glEnableVertexArrayAttrib(m_id, attrib_index);
    glVertexArrayAttribBinding(m_id, attrib_index, binding_index);
    glVertexArrayAttribFormat(m_id, attrib_index, values_per_vertex, data_type, GL_FALSE, relative_offset_in_bytes);
}

void VertexArray::bind_vertex_buffer(GLuint binding_index, GLuint vertex_buffer, GLintptr offset, GLsizei stride_in_bytes)
{
    util_assert(initialized == true, "VertexArray has not been initialized");
    glVertexArrayVertexBuffer(m_id, binding_index, vertex_buffer, offset, stride_in_bytes);
}

void VertexArray::bind_vertex_buffers(GLuint first, GLsizei count, const GLuint* buffer_ids, const GLintptr* offsets, const GLsizei* strides_in_bytes)
{
    util_assert(initialized == true, "VertexArray has not been initialized");
    glVertexArrayVertexBuffers(m_id, first, count, buffer_ids, offsets, strides_in_bytes);
}

void VertexArray::bind_element_buffer(GLuint element_buffer_id)
{
    util_assert(initialized == true, "VertexArray has not been initialized");
    glVertexArrayElementBuffer(m_id, element_buffer_id);
}

void VertexArray::binding_devisor(GLuint binding_index, GLuint divisor)
{
    util_assert(initialized == true, "VertexArray has not been initialized");
    glVertexArrayBindingDivisor(m_id, binding_index, divisor);
}

void VertexArray::bind() const
{
    util_assert(initialized == true, "VertexArray has not been initialized");
    glBindVertexArray(m_id);
}

[[nodiscard]] GLuint VertexArray::get_id() const
{
    util_assert(initialized == true, "VertexArray has not been initialized");
    return m_id;
}

} // namespace Renderer
