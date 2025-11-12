#pragma once

#include "buffer.hpp"
#include "shader.hpp"
#include "vertex.hpp"
#include "texture.hpp"

namespace Renderer {

// class Model;
// class Model::TextureRef;

class Mesh {
public:
    struct Vertex {
        glm::vec3 m_pos;
        glm::vec3 m_norm;
        glm::vec2 m_tex;
    };

    // struct Texture {
    //     uint32_t m_id;
    //     std::string m_type;
    //     std::string m_path;
    // };

    Mesh(std::vector<Vertex>&& verticies,
        std::vector<u32>&& indicies,
        std::vector<TextureRef>&& textures);
    ~Mesh() = default;

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&&) = default;
    Mesh& operator=(Mesh&&) = default;

    void draw(ShaderProgram& shader);

    std::vector<Vertex> m_verticies;
    std::vector<u32> m_indicies;
    std::vector<TextureRef> m_textures;

private:
    void setup_mesh();

    VertexArray m_vao;
    Buffer m_vbo;
    Buffer m_ebo;
};

} // namespace Renderer
