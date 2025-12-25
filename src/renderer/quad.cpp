#include "quad.hpp"

namespace Renderer {

Quad::~Quad()
{
    initialized = false;
}

void Quad::init()
{
    if (initialized) {
        throw std::runtime_error("Quad::init() attempting to reinit fullscreen quad");
    }

    m_vao.init();
    m_vao.vertex_attrib(0, 0, 3, GL_FLOAT, 0);
    m_vao.vertex_attrib(1, 0, 2, GL_FLOAT, 3 * sizeof(float));
    // clang-format off
    std::array<float, 20> quad_vertices = {
        // positions        // texture Coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };
    // clang-format on 
    m_vbo.init();
    m_vbo.buffer_data(quad_vertices.size() * sizeof(float), quad_vertices.data(), GL_STATIC_DRAW);
    m_vao.bind_vertex_buffer(0, m_vbo.get_id(), 0, 5 * sizeof(float));

    initialized = true;
}

void Quad::draw()
{
    m_vao.bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

} // namespace Renderer
