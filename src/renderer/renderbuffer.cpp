#include "renderbuffer.hpp"

namespace Renderer {

Renderbuffer::~Renderbuffer()
{
    glDeleteRenderbuffers(1, &m_id);
}

void Renderbuffer::init()
{
    glCreateRenderbuffers(1, &m_id);
}

[[nodiscard]] u32 Renderbuffer::get_id() const
{
    return m_id;
}

void Renderbuffer::buffer_storage(GLenum internal_format, GLsizei width, GLsizei height)
{
    glNamedRenderbufferStorage(m_id, internal_format, width, height);
}

void Renderbuffer::buffer_storage_multisample(GLsizei samples, GLenum internal_format, GLsizei width, GLsizei height)
{
    glNamedRenderbufferStorageMultisample(m_id, samples, internal_format, width, height);
}

} // Namespace Renderer