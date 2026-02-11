#include "mesh.hpp"

#include "model.hpp"

namespace Renderer {

Mesh::~Mesh()
{
    initialized = false;
}

void Mesh::update_model_ssbos(const std::span<glm::mat4> model_matrices)
{
    util_assert(initialized == true, "Mesh has not been initialized");

    if (!m_model_ssbo.is_initialized()) {
        m_model_ssbo.init();
        m_model_ssbo.buffer_storage(model_matrices.size() * sizeof(model_matrices[0]), model_matrices.data(), GL_DYNAMIC_STORAGE_BIT);
    } else {
        m_model_ssbo.buffer_sub_data(0, model_matrices.size() * sizeof(model_matrices[0]), model_matrices.data());
    }

    if (m_instance_count != model_matrices.size()) {
        m_instance_count = model_matrices.size();

        for (usize i = 0; i < m_commands.size(); i++) {
            m_commands.at(i).instance_count = m_instance_count;
        }

        if (Renderer::Extensions::is_extension_supported("GL_ARB_bindless_texture")) {
            m_cmd_buff.buffer_sub_data(0, m_commands.size() * sizeof(m_commands[0]), m_commands.data());
        }
    }

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_model_ssbo.get_id());
}

void Mesh::draw()
{
    util_assert(initialized == true, "Mesh has not been initialized");

    m_vao.bind();

    if (Renderer::Extensions::is_extension_supported("GL_ARB_bindless_texture")) {
        m_cmd_buff.bind_buffer(GL_DRAW_INDIRECT_BUFFER);

        glMultiDrawElementsIndirect(
            GL_TRIANGLES,
            GL_UNSIGNED_INT,
            nullptr,
            m_commands.size(),
            0);

        m_cmd_buff.unbind_buffer(GL_DRAW_INDIRECT_BUFFER);
    } else {
        for (usize i = 0; i < m_commands.size(); i++) {
            glDrawElementsInstancedBaseVertexBaseInstance(
                GL_TRIANGLES,
                m_commands[i].count,
                GL_UNSIGNED_INT,
                (void*)(m_commands[i].first_index * sizeof(GLuint)),
                m_commands[i].instance_count,
                m_commands[i].base_vertex,
                m_commands[i].base_instance);
        }
    }
}

void Mesh::draw(ShaderProgram& shader)
{
    util_assert(initialized == true, "Mesh has not been initialized");

    m_vao.bind();

    if (Renderer::Extensions::is_extension_supported("GL_ARB_bindless_texture")) {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_diff_ssbo.get_id());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_metallic_roughness_ssbo.get_id());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_normals_ssbo.get_id());
        shader.set_int("diffuse_max_textures", m_diffuse_bindless_ids.size());
        shader.set_int("metallic_roughness_max_textures", m_metallic_roughness_bindless_ids.size());
        shader.set_int("normals_max_textures", m_normal_bindless_ids.size());

        m_cmd_buff.bind_buffer(GL_DRAW_INDIRECT_BUFFER);

        glMultiDrawElementsIndirect(
            GL_TRIANGLES,
            GL_UNSIGNED_INT,
            nullptr,
            m_commands.size(),
            0);

        m_cmd_buff.unbind_buffer(GL_DRAW_INDIRECT_BUFFER);
    } else {
        for (usize i = 0; i < m_commands.size(); i++) {
            // 1 diffuse 1 metallic_roughness 1 normal 1 specular (at most.. or its broken)
            GLuint texture_unit = Texture::get_texture_unit();
            m_diffuse_textures[i]->bind(texture_unit);
            shader.set_int("tex_diffuse", static_cast<int>(texture_unit));

            texture_unit = Texture::get_texture_unit();
            m_metallic_roughness_textures[i]->bind(texture_unit);
            shader.set_int("tex_metallic_roughness", static_cast<int>(texture_unit));

            texture_unit = Texture::get_texture_unit();
            m_normal_textures[i]->bind(texture_unit);
            shader.set_int("tex_normals", static_cast<int>(texture_unit));

            glDrawElementsInstancedBaseVertexBaseInstance(
                GL_TRIANGLES,
                m_commands[i].count,
                GL_UNSIGNED_INT,
                (void*)(m_commands[i].first_index * sizeof(GLuint)),
                m_commands[i].instance_count,
                m_commands[i].base_vertex,
                m_commands[i].base_instance);

            Texture::reset_texture_units();
        }
    }
}

void Mesh::setup_mesh()
{
    util_assert(initialized == false, "Mesh::setup_mesh() has already been initialized");

    m_vao.init();
    m_vbo.init();
    m_ebo.init();

    m_vao.bind();

    m_vbo.buffer_data(static_cast<i64>(m_vertices.size() * sizeof(Vertex)), m_vertices.data(), GL_STATIC_DRAW);
    m_ebo.buffer_data(static_cast<i64>(m_indices.size() * sizeof(u32)), m_indices.data(), GL_STATIC_DRAW);

    m_vao.bind_vertex_buffer(0, m_vbo.get_id(), 0, sizeof(Vertex));
    m_vao.bind_element_buffer(m_ebo.get_id());

    m_vao.vertex_attrib(0, 0, 3, GL_FLOAT, 0);
    m_vao.vertex_attrib(1, 0, 3, GL_FLOAT, offsetof(Vertex, m_norm));
    m_vao.vertex_attrib(2, 0, 2, GL_FLOAT, offsetof(Vertex, m_tex));
    m_vao.vertex_attrib(3, 0, 3, GL_FLOAT, offsetof(Vertex, m_tang));

    m_commands.resize(m_base_vertices.size());

    for (usize i = 0; i < m_base_vertices.size(); i++) {
        m_commands[i].count = m_base_vertices.at(i).m_count;
        m_commands[i].instance_count = m_instance_count;
        m_commands[i].first_index = m_base_vertices.at(i).m_offset; // * sizeof(GLuint);
        m_commands[i].base_instance = 0;
        m_commands[i].base_vertex = m_base_vertices.at(i).m_base;
    }

    if (Renderer::Extensions::is_extension_supported("GL_ARB_bindless_texture")) {
        m_cmd_buff.init();
        m_cmd_buff.buffer_storage(m_commands.size() * sizeof(IndirectCommands), m_commands.data(), GL_DYNAMIC_STORAGE_BIT);

        m_diff_ssbo.init();
        m_metallic_roughness_ssbo.init();
        m_normals_ssbo.init();

        for (usize i = 0; i < m_commands.size(); i++) {
            // 1 diffuse 1 metallic_roughness 1 normal 1 specular (at most.. or its broken)
            m_diffuse_bindless_ids.emplace_back(m_diffuse_textures[i]->get_bindless_texture_id());
            if (!m_diffuse_textures[i]->is_bindless_texture_mapped()) {
                m_diffuse_textures[i]->map_bindless_texture();
            }

            m_metallic_roughness_bindless_ids.emplace_back(m_metallic_roughness_textures[i]->get_bindless_texture_id());
            if (!m_metallic_roughness_textures[i]->is_bindless_texture_mapped()) {
                m_metallic_roughness_textures[i]->map_bindless_texture();
            }

            m_normal_bindless_ids.emplace_back(m_normal_textures[i]->get_bindless_texture_id());
            if (!m_normal_textures[i]->is_bindless_texture_mapped()) {
                m_normal_textures[i]->map_bindless_texture();
            }
        }

        if (m_diffuse_bindless_ids.size() > 0) {
            m_diff_ssbo.buffer_storage(m_diffuse_bindless_ids.size() * sizeof(GLuint64), m_diffuse_bindless_ids.data(), GL_DYNAMIC_STORAGE_BIT);
        }
        if (m_metallic_roughness_bindless_ids.size() > 0) {
            m_metallic_roughness_ssbo.buffer_storage(m_metallic_roughness_bindless_ids.size() * sizeof(GLuint64), m_metallic_roughness_bindless_ids.data(), GL_DYNAMIC_STORAGE_BIT);
        }
        if (m_normal_bindless_ids.size() > 0) {
            m_normals_ssbo.buffer_storage(m_normal_bindless_ids.size() * sizeof(GLuint64), m_normal_bindless_ids.data(), GL_DYNAMIC_STORAGE_BIT);
        }
    }

    initialized = true;
}

} // namespace Renderer
