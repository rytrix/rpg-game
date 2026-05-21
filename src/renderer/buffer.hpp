#pragma once

namespace Renderer {

struct Buffer : public NoCopyNoMove {
    Buffer() = default;
    ~Buffer();

    void init();
    void deinit();

    void buffer_data(GLsizeiptr size, const void* data, GLenum usage);
    void buffer_storage(GLsizeiptr size, const void* data, GLbitfield flags);
    void buffer_sub_data(GLsizeiptr offset, GLsizeiptr size, const void* data);

    void bind_buffer(GLenum target) const;
    void unbind_buffer(GLenum target) const;

    [[nodiscard]] bool is_initialized() const;
    [[nodiscard]] GLuint get_id() const;

private:
    bool initialized = false;
    GLuint m_id {};
};

struct MappedBuffer : public NoCopyNoMove {
    MappedBuffer() = default;
    ~MappedBuffer();

    void init(u32 buffer_count, u32 buffer_size);
    void deinit();

    [[nodiscard]] bool is_initialized() const;
    [[nodiscard]] GLuint get_id() const;
    [[nodiscard]] void* get_ptr();
    [[nodiscard]] u32 get_buffer_size() const;

    void increment_frame();

private:
    static constexpr int FLAGS = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
    static constexpr u32 MAX_FRAME_COUNT = 3;

    bool initialized = false;
    std::array<GLuint, MAX_FRAME_COUNT> m_ids {};
    std::array<void*, MAX_FRAME_COUNT> m_ptrs {};

    u32 m_current_frame = 0;
    u32 m_frame_count = 0;
    u32 m_buffer_size = 0;
};

} // namespace Renderer
