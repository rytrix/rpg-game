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
    util_assert(initialized == false, "Mesh::Mesh() has already been initialized");

    setup_mesh();
    initialized = true;
}

void Mesh::init(std::vector<Vertex>&& verticies,
    std::vector<u32>&& indicies,
    std::vector<TextureRef>&& textures)
{
    util_assert(initialized == false, "Mesh::init() has already been initialized");

    m_verticies = std::move(verticies);
    m_indicies = std::move(indicies);
    m_textures = std::move(textures);
    setup_mesh();
    initialized = true;
}

Mesh::~Mesh()
{
    initialized = false;
}

void Mesh::draw()
{
    util_assert(initialized == true, "Mesh has not been initialized");

    m_vao.bind();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indicies.size()), GL_UNSIGNED_INT, nullptr);
}

void Mesh::draw(ShaderProgram& shader)
{
    util_assert(initialized == true, "Mesh has not been initialized");

    m_vao.bind();
    // shader.bind();
    // https://learnopengl.com/Model-Loading/Mesh
    for (int i = 0; i < m_textures.size(); i++) {
        if (strcmp(m_textures[i].m_type, "texture_diffuse") == 0) {
            GLuint texture_unit = Texture::get_texture_unit();
            m_textures[i].m_tex->m_tex.bind(texture_unit);
            shader.set_int("material.diffuse", static_cast<int>(texture_unit));

        } else if (strcmp(m_textures[i].m_type, "texture_specular") == 0) {
            GLuint texture_unit = Texture::get_texture_unit();
            m_textures[i].m_tex->m_tex.bind(texture_unit);
            shader.set_int("material.specular", static_cast<int>(texture_unit));
        }
    }

    // m_vao.bind_vertex_buffer(0, m_vbo.get_id(), 0, sizeof(Vertex));
    // m_vao.bind_element_buffer(m_ebo.get_id());
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indicies.size()), GL_UNSIGNED_INT, nullptr);
}

void Mesh::setup_mesh()
{
    m_vao.init();
    m_vbo.init();
    m_ebo.init();

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
