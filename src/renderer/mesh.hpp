#pragma once

#include "buffer.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "vertex.hpp"

namespace Renderer {

class Mesh : public NoCopyNoMove {
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

    std::vector<Vertex> m_verticies;
    std::vector<u32> m_indicies;
    std::vector<TextureRef> m_textures;

private:
    void setup_mesh();

    bool initialized = false;

    VertexArray m_vao;
    Buffer m_vbo;
    Buffer m_ebo;
};

} // namespace Renderer
