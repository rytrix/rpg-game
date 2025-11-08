#pragma once

namespace Renderer {

struct VertexArray {
    VertexArray();
    ~VertexArray();

    void vertexAttrib(GLuint attribIndex, GLuint bindingIndex, GLint valuesPerVertex, GLenum dataType, GLuint relativeOffsetInBytes);
    void bindVertexBuffer(GLuint bindingIndex, GLuint vertexBufferId, GLintptr offset, GLsizei strideInBytes);
    void bindVertexBuffers(GLuint first, GLsizei count, const GLuint* bufferIds, const GLintptr* offsets, const GLsizei* stridesInBytes);
    void bindElementBuffer(GLuint elementBufferId);
	void bindingDevisor(GLuint bindingIndex, GLuint divisor);

    void bind();

    GLuint getId() { return id; }

private:
    GLuint id;
};

} // namespace Renderer
