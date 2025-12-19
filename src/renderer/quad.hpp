#pragma once

#include "vertex.hpp"
#include "buffer.hpp"

namespace Renderer {

class Quad {
public:
    Quad();

    void init();
    void draw();

private:
    Renderer::VertexArray m_vao;
    Renderer::Buffer m_vbo;
};

} // namespace Renderer
