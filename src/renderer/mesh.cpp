#include "mesh.hpp"

#include "model.hpp"

namespace Renderer {

Mesh::Mesh(std::vector<Vertex>&& verticies,
    std::vector<u32>&& indicies,
    std::vector<TextureRef>&& textures)
    : m_vertices(std::move(verticies))
    , m_indices(std::move(indicies))
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

    m_vertices = std::move(verticies);
    m_indices = std::move(indicies);
    m_textures = std::move(textures);
    setup_mesh();
    initialized = true;
}

Mesh::~Mesh()
{
    initialized = false;
}

void Mesh::draw() const
{
    util_assert(initialized == true, "Mesh has not been initialized");

    m_vao.bind();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, nullptr);
}

void Mesh::draw(ShaderProgram& shader) const
{
    util_assert(initialized == true, "Mesh has not been initialized");

    m_vao.bind();
    // shader.bind();
    // https://learnopengl.com/Model-Loading/Mesh
    // for (usize i = 0; i < m_textures.size(); i++) {
    //     if (m_textures[i].m_type == aiTextureType_DIFFUSE) {
    //         GLuint texture_unit = Texture::get_texture_unit();
    //         m_textures[i].m_tex->bind(texture_unit);
    //         shader.set_int("material.diffuse", static_cast<int>(texture_unit));

    //     } else if (m_textures[i].m_type == aiTextureType_SPECULAR) {
    //         GLuint texture_unit = Texture::get_texture_unit();
    //         m_textures[i].m_tex->bind(texture_unit);
    //         shader.set_int("material.specular", static_cast<int>(texture_unit));
    //     }
    // }

    // glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, nullptr);

    usize texture_index = 0;
    usize offset = 0;
    for (usize i = 0; i < m_base_vertices.size(); i++) {
        // 1 diffuse 1 specular (at most.. or its broken)
        if (m_textures[texture_index].m_type == aiTextureType_DIFFUSE) {
            GLuint texture_unit = Texture::get_texture_unit();
            m_textures[texture_index].m_tex->bind(texture_unit);
            shader.set_int("material.diffuse", static_cast<int>(texture_unit));
            texture_index++;

        } else if (m_textures[texture_index].m_type == aiTextureType_SPECULAR) {
            GLuint texture_unit = Texture::get_texture_unit();
            m_textures[texture_index].m_tex->bind(texture_unit);
            shader.set_int("material.specular", static_cast<int>(texture_unit));
            texture_index++;
        }

        glDrawElementsBaseVertex(
            GL_TRIANGLES,
            m_base_vertices.at(i).count,
            GL_UNSIGNED_INT,
            (void*)offset, // WTF lol https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDrawElementsBaseVertex.xhtml
            m_base_vertices.at(i).base);

        offset += m_base_vertices.at(i).count * sizeof(GLuint);
    }
}

void Mesh::setup_mesh()
{
    m_vao.init();
    m_vbo.init();
    m_ebo.init();

    m_vao.bind();

    m_vbo.buffer_data(static_cast<i64>(m_vertices.size() * sizeof(Vertex)), &m_vertices.at(0), GL_STATIC_DRAW);
    m_ebo.buffer_data(static_cast<i64>(m_indices.size() * sizeof(u32)), &m_indices.at(0), GL_STATIC_DRAW);

    m_vao.bind_vertex_buffer(0, m_vbo.get_id(), 0, sizeof(Vertex));
    m_vao.bind_element_buffer(m_ebo.get_id());

    m_vao.vertex_attrib(0, 0, 3, GL_FLOAT, 0);
    m_vao.vertex_attrib(1, 0, 3, GL_FLOAT, offsetof(Vertex, m_norm));
    m_vao.vertex_attrib(2, 0, 2, GL_FLOAT, offsetof(Vertex, m_tex));

    initialized = true;
}

} // namespace Renderer
