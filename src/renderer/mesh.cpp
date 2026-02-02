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

void Mesh::draw()
{
    util_assert(initialized == true, "Mesh has not been initialized");

    m_vao.bind();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, nullptr);
}

void Mesh::draw(ShaderProgram& shader)
{
    util_assert(initialized == true, "Mesh has not been initialized");

    m_vao.bind();
    // usize texture_index = 0;
    // usize draw_index = 0;

    // std::vector<GLuint> diffuse_textures;
    // std::vector<GLuint> specular_textures;

    // for (usize i = 0; i < m_commands.size(); i++) {
    //     // 1 diffuse 1 specular (at most.. or its broken)
    //     if (m_textures[texture_index].m_type == aiTextureType_DIFFUSE) {
    //         GLuint texture_unit = Texture::get_texture_unit();
    //         m_textures[texture_index].m_tex->bind(texture_unit);
    //         // diffuse_textures.emplace_back(texture_unit);
    //         // shader.set_int(std::format("material.diffuse[{}]", draw_index).c_str(), static_cast<int>(texture_unit));
    //         texture_index++;
    //     } else if (m_textures[texture_index].m_type == aiTextureType_SPECULAR) {
    //         GLuint texture_unit = Texture::get_texture_unit();
    //         m_textures[texture_index].m_tex->bind(texture_unit);
    //         // textures.emplace_back(texture_unit);
    //         shader.set_int(std::format("material.specular[{}]", draw_index).c_str(), static_cast<int>(texture_unit));
    //         texture_index++;
    //     }
    //     draw_index++;

    //     // glDrawElementsInstancedBaseVertexBaseInstance(
    //     //     GL_TRIANGLES,
    //     //     m_commands[i].count,
    //     //     GL_UNSIGNED_INT,
    //     //     (void*)m_commands[i].first_index * sizeof(GLuint),
    //     //     m_commands[i].instance_count,
    //     //     m_commands[i].base_vertex,
    //     //     m_commands[i].base_instance);
    // }

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_diff_ssbo.get_id());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_spec_ssbo.get_id());
    shader.set_int("diffuse_max_textures", m_diffuse_textures.size());
    shader.set_int("specular_max_textures", m_specular_textures.size());

    m_cmd_buff.bind_buffer(GL_DRAW_INDIRECT_BUFFER);

    glMultiDrawElementsIndirect(
        GL_TRIANGLES,
        GL_UNSIGNED_INT,
        nullptr,
        m_commands.size(),
        0);

    m_cmd_buff.unbind_buffer(GL_DRAW_INDIRECT_BUFFER);
}

void Mesh::setup_mesh()
{
    m_vao.init();
    m_vbo.init();
    m_ebo.init();
    m_cmd_buff.init();
    m_diff_ssbo.init();
    m_spec_ssbo.init();

    m_vao.bind();

    m_vbo.buffer_data(static_cast<i64>(m_vertices.size() * sizeof(Vertex)), m_vertices.data(), GL_STATIC_DRAW);
    m_ebo.buffer_data(static_cast<i64>(m_indices.size() * sizeof(u32)), m_indices.data(), GL_STATIC_DRAW);

    m_vao.bind_vertex_buffer(0, m_vbo.get_id(), 0, sizeof(Vertex));
    m_vao.bind_element_buffer(m_ebo.get_id());

    m_vao.vertex_attrib(0, 0, 3, GL_FLOAT, 0);
    m_vao.vertex_attrib(1, 0, 3, GL_FLOAT, offsetof(Vertex, m_norm));
    m_vao.vertex_attrib(2, 0, 2, GL_FLOAT, offsetof(Vertex, m_tex));

    m_commands.resize(m_base_vertices.size());

    usize offset = 0;
    for (usize i = 0; i < m_base_vertices.size(); i++) {
        m_commands[i].count = m_base_vertices.at(i).m_count;
        m_commands[i].instance_count = 1;
        m_commands[i].first_index = offset; // * sizeof(GLuint);
        m_commands[i].base_instance = 0;
        m_commands[i].base_vertex = m_base_vertices.at(i).m_base;

        offset += m_base_vertices.at(i).m_count;
    }

    m_cmd_buff.buffer_storage(m_commands.size() * sizeof(IndirectCommands), m_commands.data(), 0);

    usize texture_index = 0;
    for (usize i = 0; i < m_commands.size(); i++) {
        // 1 diffuse 1 specular (at most.. or its broken)
        if (m_textures[texture_index].m_type == aiTextureType_DIFFUSE) {
            m_diffuse_textures.emplace_back(m_textures[texture_index].m_tex->get_bindless_texture_id());
            if (!m_textures[texture_index].m_tex->is_bindless_texture_mapped()) {
                m_textures[texture_index].m_tex->map_bindless_texture();
            }
            texture_index++;
        } else if (m_textures[texture_index].m_type == aiTextureType_SPECULAR) {
            m_specular_textures.emplace_back(m_textures[texture_index].m_tex->get_bindless_texture_id());
            if (!m_textures[texture_index].m_tex->is_bindless_texture_mapped()) {
                m_textures[texture_index].m_tex->map_bindless_texture();
            }
            texture_index++;
        }
    }

    if (m_diffuse_textures.size() > 0) {
        m_diff_ssbo.buffer_storage(m_diffuse_textures.size() * sizeof(GLuint64), m_diffuse_textures.data(), GL_DYNAMIC_STORAGE_BIT);
    }
    if (m_specular_textures.size() > 0) {
        m_spec_ssbo.buffer_storage(m_specular_textures.size() * sizeof(GLuint64), m_specular_textures.data(), GL_DYNAMIC_STORAGE_BIT);
    }

    initialized = true;
}

} // namespace Renderer
