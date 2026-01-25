#pragma once

#include "buffer.hpp"
#include "vertex.hpp"

namespace Renderer {

class Quad : public NoCopyNoMove {
public:
    Quad() = default;
    ~Quad();

    void init();
    void draw();

private:
    bool initialized = false;

    Renderer::VertexArray m_vao;
    Renderer::Buffer m_vbo;
};

} // namespace Renderer
