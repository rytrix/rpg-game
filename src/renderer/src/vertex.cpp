#include "vertex.hpp"

namespace Renderer {

VertexArray::VertexArray()
{
    glCreateVertexArrays(1, &id);
	glBindVertexArray(id);
}

VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &id);
}

void VertexArray::vertexAttrib(GLuint attribIndex, GLuint bindingIndex, GLint valuesPerVertex, GLenum dataType, GLuint relativeOffsetInBytes)
{
    glEnableVertexArrayAttrib(id, attribIndex);
    glVertexArrayAttribBinding(id, attribIndex, bindingIndex);
    glVertexArrayAttribFormat(id, attribIndex, valuesPerVertex, dataType, GL_FALSE, relativeOffsetInBytes);
}

void VertexArray::bindVertexBuffer(GLuint bindingIndex, GLuint vertexBuffer, GLintptr offset, GLsizei strideInBytes)
{
	glVertexArrayVertexBuffer(id, bindingIndex, vertexBuffer, offset, strideInBytes);
}

void VertexArray::bindVertexBuffers(GLuint first, GLsizei count, const GLuint *bufferIds, const GLintptr *offsets, const GLsizei *stridesInBytes)
{
	glVertexArrayVertexBuffers(id, first, count, bufferIds, offsets, stridesInBytes);
}

void VertexArray::bindElementBuffer(GLuint elementBufferId)
{
	glVertexArrayElementBuffer(id, elementBufferId);
}

void VertexArray::bindingDevisor(GLuint bindingIndex, GLuint divisor)
{
	glVertexArrayBindingDivisor(id, bindingIndex, divisor);
}

void VertexArray::bind()
{
	glBindVertexArray(id);
}

} // namespace Renderer
