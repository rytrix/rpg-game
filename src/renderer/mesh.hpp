#pragma once

#include "assimp/material.h"
#include "buffer.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "vertex.hpp"

namespace Renderer {

struct TextureRef {
    Texture* m_tex;
    aiTextureType m_type;
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
    };

    Mesh() = default;
    Mesh(std::vector<Vertex>&& verticies,
        std::vector<u32>&& indicies,
        std::vector<TextureRef>&& textures);
    ~Mesh();

    void init(std::vector<Vertex>&& verticies,
        std::vector<u32>&& indicies,
        std::vector<TextureRef>&& textures);

    void draw() const;
    void draw(ShaderProgram& shader) const;

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
};

} // namespace Renderer
