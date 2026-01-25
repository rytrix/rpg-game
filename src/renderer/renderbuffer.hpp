#pragma once

namespace Renderer {

class Renderbuffer : public NoCopyNoMove {
public:
    Renderbuffer() = default;
    ~Renderbuffer();

    void init();

    [[nodiscard]] u32 get_id() const;

    void buffer_storage(GLenum internal_format, GLsizei width, GLsizei height);
    void buffer_storage_multisample(GLsizei samples, GLenum internal_format, GLsizei width, GLsizei height);

private:
    bool initialized = false;

    u32 m_id {};
};

}