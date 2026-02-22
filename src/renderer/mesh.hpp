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
    struct Vertex {
        glm::vec3 m_pos;
        glm::vec3 m_norm;
        glm::vec2 m_tex;
        glm::vec3 m_tang;
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
