#pragma once

#include "animation.hpp"
#include "buffer.hpp"
#include "extensions.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "vertex.hpp"

#include "../app_data.hpp"

namespace Renderer {

struct IndirectCommands {
    GLuint count;
    GLuint instance_count;
    GLuint first_index;
    GLint base_vertex;
    GLuint base_instance;
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

static constexpr u32 MAX_BONES = 150;
static constexpr u32 MAX_BONES_PER_VERTEX = 4;

static constexpr std::string get_bone_defines()
{
    return std::format(
        "#define ENABLE_BONES\n#define BONES_PER_VERTEX {}\n#define MAX_BONES {}\n",
        MAX_BONES_PER_VERTEX, MAX_BONES);
}

struct VertexBone {
    VertexBone();
    void add_bone(const u32 bone_id, const float weight);

    std::array<u32, MAX_BONES_PER_VERTEX> bones {};
    std::array<float, MAX_BONES_PER_VERTEX> weights {};
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

    struct VertexData {
        std::vector<Vertex> m_vertices;
        std::vector<VertexBone> m_bones;
        std::vector<u32> m_indices;
    };

    Mesh() = default;
    ~Mesh();

    void update_model_ssbos(const std::span<glm::mat4> model_matrices);
    void update_bone_matrices(const std::span<PerAnimationData*> animation_data);

    void draw_untextured(Renderer::ShaderProgram& shader);
    void draw(ShaderProgram& shader);

    VertexData m_vertex_data;

    std::vector<Handle> m_diffuse_textures;
    std::vector<Handle> m_metallic_roughness_textures;
    std::vector<Handle> m_normal_textures;

    bool m_has_bones = false;
    std::unordered_map<Utils::String, u32> m_bone_id_map;

    Animation* m_animation = nullptr;
    GlobalAppData* m_app_data = nullptr;

    std::vector<BaseVertex> m_base_vertices;

private:
    void setup_mesh();

    bool initialized = false;

    VertexArray m_vao;
    Buffer m_vbo;
    Buffer m_ebo;

    Buffer m_bones_vbo;

    Buffer m_cmd_buff;
    std::vector<IndirectCommands> m_commands;

    Buffer m_model_ssbo;
    GLuint m_instance_count = 1;

    Buffer m_texture_ssbo;
    std::vector<GLuint64> m_texture_bindless_ids;

    Buffer m_bone_ssbo;
};

} // namespace Renderer
