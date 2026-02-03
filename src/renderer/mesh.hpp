#pragma once

#include "assimp/material.h"
#include "buffer.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "vertex.hpp"
#include "extensions.hpp"

namespace Renderer {

struct TextureRef {
    Texture* m_tex;
    aiTextureType m_type;
};

#pragma pack(push, 1)
struct IndirectCommands {
    GLuint count;
    GLuint instance_count;
    GLuint first_index;
    GLint base_vertex;
    GLuint base_instance;
};
#pragma pack(pop)

class Mesh : public NoCopyNoMove {
    friend class Model;

public:
    struct Vertex {
        glm::vec3 m_pos;
        glm::vec3 m_norm;
        glm::vec2 m_tex;
    };

    Mesh() = default;
    Mesh(std::vector<Vertex>&& verticies,
        std::vector<u32>&& indicies,
        std::vector<TextureRef>&& textures);
    ~Mesh();

    void init(std::vector<Vertex>&& verticies,
        std::vector<u32>&& indicies,
        std::vector<TextureRef>&& textures);

    void draw();
    void draw(ShaderProgram& shader);

    std::vector<Vertex> m_vertices;
    std::vector<u32> m_indices;
    std::vector<TextureRef> m_textures;

    struct BaseVertex {
        GLsizei m_count;
        GLsizei m_base;

        BaseVertex(GLsizei count, GLsizei base)
            : m_count(count)
            , m_base(base)
        {
        }
    };
    std::vector<BaseVertex> m_base_vertices;

private:
    void setup_mesh();

    bool initialized = false;

    VertexArray m_vao;
    Buffer m_vbo;
    Buffer m_ebo;

    Buffer m_cmd_buff;
    Buffer m_diff_ssbo;
    Buffer m_spec_ssbo;

    std::vector<IndirectCommands> m_commands;
    std::vector<GLuint64> m_diffuse_textures;
    std::vector<GLuint64> m_specular_textures;
};

} // namespace Renderer
