#include "mesh.hpp"

#include "model.hpp"

namespace Renderer {

Mesh::Mesh(std::vector<Vertex>&& verticies,
    std::vector<u32>&& indicies,
    std::vector<TextureRef>&& textures)
    : m_verticies(std::move(verticies))
    , m_indicies(std::move(indicies))
    , m_textures(std::move(textures))
{
    setup_mesh();
}

void Mesh::draw(ShaderProgram& shader)
{
    shader.bind();

    // TODO
    // Do something with textures
    // shader.set_int(const char *name, int value)
    // https://learnopengl.com/Model-Loading/Mesh

    m_vao.bind();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indicies.size()), GL_UNSIGNED_INT, nullptr);
}

void Mesh::setup_mesh()
{
    m_vao.bind();

    m_vbo.buffer_data(static_cast<i64>(m_verticies.size() * sizeof(Vertex)), &m_verticies.at(0), GL_STATIC_DRAW);
    m_ebo.buffer_data(static_cast<i64>(m_indicies.size() * sizeof(u32)), &m_indicies.at(0), GL_STATIC_DRAW);

    m_vao.bind_vertex_buffer(0, m_vbo.get_id(), 0, sizeof(Vertex));
    m_vao.bind_element_buffer(m_ebo.get_id());

    m_vao.vertex_attrib(0, 0, 3, GL_FLOAT, 0);
    m_vao.vertex_attrib(1, 0, 3, GL_FLOAT, offsetof(Vertex, m_norm));
    m_vao.vertex_attrib(2, 0, 2, GL_FLOAT, offsetof(Vertex, m_tex));
}

} // namespace Renderer
