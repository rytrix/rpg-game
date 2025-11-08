#include "vertex.hpp"

namespace Renderer {

VertexArray::VertexArray()
{
    glCreateVertexArrays(1, &m_id);
    glBindVertexArray(m_id);
}

VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &m_id);
}

void VertexArray::vertex_attrib(GLuint attrib_index, GLuint binding_index, GLint values_per_vertex, GLenum data_type, GLuint relative_offset_in_bytes)
{
    glEnableVertexArrayAttrib(m_id, attrib_index);
    glVertexArrayAttribBinding(m_id, attrib_index, binding_index);
    glVertexArrayAttribFormat(m_id, attrib_index, values_per_vertex, data_type, GL_FALSE, relative_offset_in_bytes);
}

void VertexArray::bind_vertex_buffer(GLuint binding_index, GLuint vertex_buffer, GLintptr offset, GLsizei stride_in_bytes)
{
    glVertexArrayVertexBuffer(m_id, binding_index, vertex_buffer, offset, stride_in_bytes);
}

void VertexArray::bind_vertex_buffers(GLuint first, GLsizei count, const GLuint* buffer_ids, const GLintptr* offsets, const GLsizei* strides_in_bytes)
{
    glVertexArrayVertexBuffers(m_id, first, count, buffer_ids, offsets, strides_in_bytes);
}

void VertexArray::bind_element_buffer(GLuint element_buffer_id)
{
    glVertexArrayElementBuffer(m_id, element_buffer_id);
}

void VertexArray::binding_devisor(GLuint binding_index, GLuint divisor)
{
    glVertexArrayBindingDivisor(m_id, binding_index, divisor);
}

void VertexArray::bind()
{
    glBindVertexArray(m_id);
}

[[nodiscard]] GLuint VertexArray::get_id() const
{
    return m_id;
}

} // namespace Renderer
