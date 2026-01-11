#include "renderbuffer.hpp"

namespace Renderer {

Renderbuffer::~Renderbuffer()
{
    if (initialized) {
        glDeleteRenderbuffers(1, &m_id);
        initialized = false;
    }
}

void Renderbuffer::init()
{
    util_assert(initialized == false, "Renderbuffer::init() has already been initialized");
    glCreateRenderbuffers(1, &m_id);
    initialized = true;
}

[[nodiscard]] u32 Renderbuffer::get_id() const
{
    util_assert(initialized == true, "Renderbuffer has not been initialized");
    return m_id;
}

void Renderbuffer::buffer_storage(GLenum internal_format, GLsizei width, GLsizei height)
{
    util_assert(initialized == true, "Renderbuffer has not been initialized");
    glNamedRenderbufferStorage(m_id, internal_format, width, height);
}

void Renderbuffer::buffer_storage_multisample(GLsizei samples, GLenum internal_format, GLsizei width, GLsizei height)
{
    util_assert(initialized == true, "Renderbuffer has not been initialized");
    glNamedRenderbufferStorageMultisample(m_id, samples, internal_format, width, height);
}

} // Namespace Renderer