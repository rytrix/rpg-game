#pragma once

#include "buffer.hpp"
#include "vertex.hpp"

namespace Renderer {

class Quad {
public:
    Quad() = default;
    ~Quad();

    Quad(const Quad&) = delete;
    Quad& operator=(const Quad&) = delete;
    Quad(Quad&&) = default;
    Quad& operator=(Quad&&) = default;

    void init();
    void draw();

private:
    bool initialized = false;

    Renderer::VertexArray m_vao;
    Renderer::Buffer m_vbo;
};

} // namespace Renderer
