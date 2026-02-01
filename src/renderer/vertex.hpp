#pragma once

namespace Renderer {

class VertexArray : public NoCopyNoMove {
public:
    VertexArray() = default;
    ~VertexArray();

    void init();

    void vertex_attrib(GLuint attrib_index, GLuint binding_index, GLint values_per_vertex, GLenum data_type, GLuint relative_offset_in_bytes);
    void bind_vertex_buffer(GLuint binding_index, GLuint vertex_buffer_id, GLintptr offset, GLsizei stride_in_bytes);
    void bind_vertex_buffers(GLuint first, GLsizei count, const GLuint* buffer_ids, const GLintptr* offsets, const GLsizei* strides_in_bytes);
    void bind_element_buffer(GLuint element_buffer_id);
    void bind_indirect_buffer(GLuint indirect_buffer_id);
    void binding_devisor(GLuint binding_index, GLuint divisor);

    void bind() const;

    [[nodiscard]] GLuint get_id() const;

private:
    bool initialized = false;

    GLuint m_id {};
};

} // namespace Renderer
