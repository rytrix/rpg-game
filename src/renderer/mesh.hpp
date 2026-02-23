#pragma once

#include "assimp/material.h"
#include "buffer.hpp"
#include "extensions.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "vertex.hpp"

namespace Renderer {

struct TextureRef {
    Texture* m_tex = nullptr;
    aiTextureType m_type = aiTextureType_DIFFUSE;
};

struct IndirectCommands {
    GLuint count;
    GLuint instance_count;
    GLuint first_index;
    GLint base_vertex;
    GLuint base_instance;
};

class Mesh : public NoCopyNoMove {
    friend class Model;

public:
    static constexpr u32 MAX_VERTEX_BONES = 4;

    struct Vertex {
        glm::vec3 m_pos;
        glm::vec3 m_norm;
        glm::vec2 m_tex;
        glm::vec3 m_tang;
    };

    struct VertexBone {
        std::array<u32, MAX_VERTEX_BONES> bones {};
        std::array<float, MAX_VERTEX_BONES> weights {};

        void add_bone(const u32 bone_id, const float weight)
        {
            for (u32 i = 0; i < MAX_VERTEX_BONES; i++) {
                if (weights.at(i) == 0.0F) {
                    bones.at(i) = bone_id;
                    weights.at(i) = weight;
                    return;
                }
            }
            util_error("Exceded max number of bones per vertex");
        }
    };

    struct BoneInfo {
        glm::mat4 transform;
    };

    struct BaseVertex {
        GLsizei m_count {};
        GLsizei m_base {};
        GLuint m_offset {};
        GLuint m_base_bone {};

        BaseVertex(GLsizei count, GLsizei base, GLuint base_bone)
            : m_count(count)
            , m_base(base)
            , m_base_bone(base_bone)
        {
        }
    };

    Mesh() = default;
    ~Mesh();

    void update_model_ssbos(const std::span<glm::mat4> model_matrices);

    void draw();
    void draw(ShaderProgram& shader);

    std::vector<Vertex> m_vertices;
    std::vector<u32> m_indices;

    std::vector<Texture*> m_diffuse_textures;
    std::vector<Texture*> m_metallic_roughness_textures;
    std::vector<Texture*> m_normal_textures;

    std::unordered_map<const char*, u32> m_bone_id_map;
    std::vector<VertexBone> m_vertex_bones;
    std::vector<BoneInfo> m_bones;

    std::vector<BaseVertex> m_base_vertices;

private:
    void setup_mesh();

    bool initialized = false;

    VertexArray m_vao;
    Buffer m_vbo;
    Buffer m_ebo;

    // Indirect info
    Buffer m_cmd_buff;
    std::vector<IndirectCommands> m_commands;

    Buffer m_model_ssbo;
    GLuint m_instance_count = 1;

    Buffer m_texture_ssbo;
    std::vector<GLuint64> m_texture_bindless_ids;
};

} // namespace Renderer
