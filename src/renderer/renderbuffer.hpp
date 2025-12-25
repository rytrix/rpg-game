#pragma once

namespace Renderer {

class Renderbuffer {
public:
    Renderbuffer() = default;
    ~Renderbuffer();

    Renderbuffer(const Renderbuffer&) = delete;
    Renderbuffer& operator=(const Renderbuffer&) = delete;
    Renderbuffer(Renderbuffer&&) = default;
    Renderbuffer& operator=(Renderbuffer&&) = default;

    void init();

    [[nodiscard]] u32 get_id() const;

    void buffer_storage(GLenum internal_format, GLsizei width, GLsizei height);
    void buffer_storage_multisample(GLsizei samples, GLenum internal_format, GLsizei width, GLsizei height);

private:
    bool initialized = false;

    u32 m_id {};
};

}