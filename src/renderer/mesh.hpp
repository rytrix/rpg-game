#pragma once

#include "../utils/math/aabb.hpp"

#include "animation.hpp"
#include "buffer.hpp"
#include "extensions.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "vertex.hpp"

#include "../scene/resource_manager.hpp"

class GlobalAppData;

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
    friend class ModelLoader;

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

    Mesh(const char* path, GlobalAppData* app_data);
    Mesh() = default;
    ~Mesh();

    void update(u32 instance_count, const std::span<glm::mat4> transform_matrices, const std::span<AnimationData*> animation_data);

    void draw_untextured(Renderer::Shader& shader);
    void draw(Shader& shader);

    VertexData m_vertex_data;

    std::vector<Handle> m_diffuse_textures;
    std::vector<Handle> m_metallic_roughness_textures;
    std::vector<Handle> m_normal_textures;

    std::vector<Utils::AABB> m_aabbs;

    bool m_has_bones = false;
    std::unordered_map<Utils::String, u32> m_bone_id_map;

    GlobalAppData* m_app_data = nullptr;

    std::vector<BaseVertex> m_base_vertices;

    std::deque<Animation> m_animations;

private:
    void setup_mesh();

    void update_instance_count(u32 instance_count);
    void update_model_ssbos(const std::span<glm::mat4> model_matrices);
    void update_bone_matrices(const std::span<AnimationData*> animation_data);
    void next_ssbo_frame();

    bool initialized = false;

    VertexArray m_vao;
    Buffer m_vbo;
    Buffer m_ebo;
    Buffer m_bones_vbo;

    Buffer m_texture_ssbo;
    std::vector<GLuint64> m_texture_bindless_ids;

    Buffer m_cmd_buff;
    std::vector<IndirectCommands> m_commands;

    GLuint m_instance_count = 1;
    MappedBuffer m_model_ssbo;
    MappedBuffer m_bone_ssbo;
};

} // namespace Renderer
