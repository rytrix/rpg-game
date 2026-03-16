#include "mesh.hpp"

#include "model.hpp"

#include "../utils/helpers.hpp"

namespace Renderer {

VertexBone::VertexBone()
{
    for (auto& bone : bones) {
        // bone = UINT32_MAX;
        bone = 0;
    }
}

void VertexBone::add_bone(const u32 bone_id, const float weight)
{
    for (u32 i = 0; i < MAX_BONES_PER_VERTEX; i++) {
        if (weights.at(i) == 0.0F) {
            // if (bones.at(i) == UINT32_MAX) {
            bones.at(i) = bone_id;
            weights.at(i) = weight;
            return;
        }
    }
    util_error("Exceded max number of bones per vertex");
}

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
        if (model_matrices.size() > m_instance_count) {
            m_model_ssbo.~Buffer();
            m_model_ssbo.init();
            m_model_ssbo.buffer_storage(model_matrices.size() * sizeof(model_matrices[0]), model_matrices.data(), GL_DYNAMIC_STORAGE_BIT);
        } else {
            m_model_ssbo.buffer_sub_data(0, model_matrices.size() * sizeof(model_matrices[0]), model_matrices.data());
        }
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
}

void Mesh::evaluate_bone_matrices(double animation_time, const aiNode* node, const glm::mat4& parent_transform)
{
    util_assert(initialized == true, "Mesh has not been initialized");
    util_assert(m_has_bones == true, "Attempting to update bone matrices with no bones");

    glm::mat4 node_transform = mat4_to_mat4(node->mTransformation);
    glm::mat4 global_transform;

    if (m_bone_id_map.contains(node->mName.C_Str())) {
        u32 index = m_bone_id_map[node->mName.C_Str()];

        // If animated
        if (m_bones[index].m_animation.is_initialized()) {
            node_transform = m_bones[index].m_animation.keyframe_to_mat4(animation_time);
        }

        global_transform = parent_transform * node_transform;

        m_final_bone_matrices[index] = m_global_inverse_transform * global_transform * m_bones[index].m_offset;
    } else {
        global_transform = parent_transform * node_transform;
    }

    for (u32 i = 0; i < node->mNumChildren; i++) {
        evaluate_bone_matrices(animation_time, node->mChildren[i], global_transform);
    }
}

void Mesh::update_bone_matrices(double animation_time)
{
    util_assert(initialized == true, "Mesh has not been initialized");
    util_assert(m_has_bones == true, "Attempting to update bone matrices with no bones");

    animation_time = std::fmod(animation_time * m_ticks_per_second, m_total_animation_time);

    util_assert(m_bones.size() <= MAX_BONES,
        std::format("Exceded max bone limit of {} with {} bones",
            MAX_BONES,
            m_bones.size()));
    m_final_bone_matrices.resize(m_bones.size());

    glm::mat4 identity(1.0F);
    evaluate_bone_matrices(animation_time, m_scene->mRootNode, identity);

    m_bone_ssbo.buffer_sub_data(0,
        static_cast<GLsizeiptr>(sizeof(glm::mat4) * m_final_bone_matrices.size()),
        m_final_bone_matrices.data());
}

void Mesh::draw_untextured(Renderer::ShaderProgram& shader)
{
    util_assert(initialized == true, "Mesh has not been initialized");

    m_vao.bind();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_model_ssbo.get_id());
    if (m_has_bones) {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_bone_ssbo.get_id());
    }
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

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_model_ssbo.get_id());
    if (m_has_bones) {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_bone_ssbo.get_id());
    }
    if (Renderer::Extensions::is_extension_supported("GL_ARB_bindless_texture")) {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_texture_ssbo.get_id());

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

            Texture::drop_texture_units(3);
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

    if (m_has_bones) {
        m_bones_vbo.init();
        m_bones_vbo.buffer_data(static_cast<i64>(m_vertex_bones.size() * sizeof(VertexBone)), m_vertex_bones.data(), GL_STATIC_DRAW);
        m_vao.bind_vertex_buffer(1, m_bones_vbo.get_id(), 0, sizeof(VertexBone));

        m_vao.vertex_attrib_int(4, 1, MAX_BONES_PER_VERTEX, GL_INT, 0);
        m_vao.vertex_attrib(5, 1, MAX_BONES_PER_VERTEX, GL_FLOAT, MAX_BONES_PER_VERTEX * sizeof(GLint));
    }

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

        m_texture_ssbo.init();
        m_texture_bindless_ids.resize(m_commands.size() * 3);
        for (usize i = 0; i < m_commands.size(); i++) {
            // 1 diffuse 1 metallic_roughness 1 normal (at most.. or its broken)
            m_texture_bindless_ids.at((i * 3) + 0) = m_diffuse_textures[i]->get_bindless_texture_id();
            if (!m_diffuse_textures[i]->is_bindless_texture_mapped()) {
                m_diffuse_textures[i]->map_bindless_texture();
            }

            m_texture_bindless_ids.at((i * 3) + 1) = m_metallic_roughness_textures[i]->get_bindless_texture_id();
            if (!m_metallic_roughness_textures[i]->is_bindless_texture_mapped()) {
                m_metallic_roughness_textures[i]->map_bindless_texture();
            }

            m_texture_bindless_ids.at((i * 3) + 2) = m_normal_textures[i]->get_bindless_texture_id();
            if (!m_normal_textures[i]->is_bindless_texture_mapped()) {
                m_normal_textures[i]->map_bindless_texture();
            }
        }
        if (m_texture_bindless_ids.size() > 0) {
            m_texture_ssbo.buffer_storage(m_texture_bindless_ids.size() * sizeof(GLuint64), m_texture_bindless_ids.data(), 0);
        }
    }

    if (m_has_bones) {
        m_bone_ssbo.init();
        m_bone_ssbo.buffer_storage(sizeof(glm::mat4) * MAX_BONES, nullptr, GL_DYNAMIC_STORAGE_BIT);
    }

    initialized = true;
}

} // namespace Renderer
