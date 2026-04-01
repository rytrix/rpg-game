#pragma once

#include "../utils/string.hpp"

#include "buffer.hpp"
#include "vertex.hpp"

struct Handle;

namespace Renderer {

static constexpr u32 MAX_BONES_PER_VERTEX = 4;

struct Vertex {
    glm::vec3 m_pos;
    glm::vec3 m_norm;
    glm::vec2 m_tex;
    glm::vec3 m_tang;
};

struct VertexBone {
    std::array<u32, MAX_BONES_PER_VERTEX> m_bones {};
    std::array<float, MAX_BONES_PER_VERTEX> m_weights {};
};

struct BaseVertex;

struct MeshData {
    std::vector<Vertex> m_verticies;
    std::vector<VertexBone> m_bones;
    std::vector<u32> m_indices;

    std::vector<Handle> m_diffuse_textures;
    std::vector<Handle> m_metallic_roughness_textures;
    std::vector<Handle> m_normal_textures;

    std::vector<BaseVertex> m_base_vertices;
    GLuint m_instance_count = 1;

    std::unordered_map<Utils::String, u32> m_bone_id_map;
};

struct IndirectCommands {
    GLuint count;
    GLuint instance_count;
    GLuint first_index;
    GLint base_vertex;
    GLuint base_instance;
};

struct MeshOpenGL {
    VertexArray m_vao;
    Buffer m_vbo;
    Buffer m_bones_vbo;
    Buffer m_ebo;

    Buffer m_cmd_buff;
    std::vector<IndirectCommands> m_commands;

    Buffer m_model_ssbo;

    Buffer m_texture_ssbo;
    std::vector<GLuint64> m_texture_bindless_ids;

    Buffer m_bone_ssbo;
};

} // namespace Renderer