#pragma once

#include "animation.hpp"
#include "assimp/material.h"
#include "buffer.hpp"
#include "extensions.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "vertex.hpp"

namespace Renderer {

struct IndirectCommands {
    GLuint count;
    GLuint instance_count;
    GLuint first_index;
    GLint base_vertex;
    GLuint base_instance;
};

static constexpr u32 MAX_BONES = 100;
static constexpr u32 MAX_BONES_PER_VERTEX = 4;

static constexpr std::string get_bone_defines()
{
    return std::format(
        "#define ENABLE_BONES\n#define MAX_BONES {}\n#define BONES_PER_VERTEX {}\n",
        MAX_BONES,
        MAX_BONES_PER_VERTEX);
}

struct VertexBone {
    std::array<u32, MAX_BONES_PER_VERTEX> bones {};
    std::array<float, MAX_BONES_PER_VERTEX> weights {};

    VertexBone()
    {
        for (auto& bone : bones) {
            // bone = UINT32_MAX;
            bone = 0;
        }
    }

    void add_bone(const u32 bone_id, const float weight)
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
};

struct BoneInfo {
    glm::mat4 m_offset;
    BoneAnimation m_animation;

    BoneInfo(glm::mat4 offset, aiNodeAnim* anim)
        : m_offset(offset)
        , m_animation(anim)
    {
    }
};

struct BaseVertex {
    GLsizei m_count {};
    GLsizei m_base {};
    GLuint m_offset {};

    BaseVertex(GLsizei count, GLsizei base)
        : m_count(count)
        , m_base(base)
    {
    }
};

class Mesh : public NoCopyNoMove {
    friend class Model;

public:
    struct Vertex {
        glm::vec3 m_pos;
        glm::vec3 m_norm;
        glm::vec2 m_tex;
        glm::vec3 m_tang;
    };

    Mesh() = default;
    ~Mesh();

    void update_model_ssbos(const std::span<glm::mat4> model_matrices);
    void update_bone_matrices(const double animation_time);

    void draw_untextured(Renderer::ShaderProgram& shader);
    void draw(ShaderProgram& shader);

    std::vector<Vertex> m_vertices;
    std::vector<VertexBone> m_vertex_bones;
    std::vector<u32> m_indices;

    std::vector<Texture*> m_diffuse_textures;
    std::vector<Texture*> m_metallic_roughness_textures;
    std::vector<Texture*> m_normal_textures;

    bool m_has_bones = false;
    std::unordered_map<std::string, u32> m_bone_id_map;
    std::vector<BoneInfo> m_bones;
    std::vector<glm::mat4> m_final_bone_matrices;
    glm::mat4 m_global_inverse_transform;

    const aiScene* m_scene = nullptr;

    std::vector<BaseVertex> m_base_vertices;

    double m_total_animation_time = 0.0;
    double m_ticks_per_second = 0.0;

private:
    void setup_mesh();
    void evaluate_bone_matrices(double animation_time, const aiNode* node, const glm::mat4& parent_transform);

    bool initialized = false;

    VertexArray m_vao;
    Buffer m_vbo;
    Buffer m_ebo;

    Buffer m_bones_vbo;

    // Indirect info
    Buffer m_cmd_buff;
    std::vector<IndirectCommands> m_commands;

    Buffer m_model_ssbo;
    GLuint m_instance_count = 1;

    Buffer m_texture_ssbo;
    std::vector<GLuint64> m_texture_bindless_ids;

    Buffer m_bone_ssbo;
};

} // namespace Renderer
