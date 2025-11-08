#pragma once

namespace Renderer {

struct Buffer {
    Buffer();
    ~Buffer();

    void bufferData(GLsizeiptr size, const void* data, GLenum usage);
	void bufferStorage(GLsizeiptr size, const void* data, GLbitfield flags);
	void bufferSubData(GLsizeiptr offset, GLsizeiptr size, const void* data);

	GLuint getId() const noexcept { return id; }

private:
    GLuint id;
};

} // namespace Renderer
