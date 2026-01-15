#pragma once

#include "mesh.hpp"
#include "shader.hpp"
#include "texture.hpp"

#include <assimp/scene.h>

namespace Renderer {

class Model {
public:
    Model() = default;
    explicit Model(const char* path);
    ~Model();

    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;
    Model(Model&&) noexcept = default;
    Model& operator=(Model&&) = default;

    void init(const char* path);

    void draw();
    void draw(ShaderProgram& shader);

    const std::deque<Mesh>* get_meshes();

private:
    bool initialized = false;

    std::deque<TextureStorage> m_textures;
    std::deque<Mesh> m_meshes;
    std::string m_directory;

    void process_node(aiNode* node, const aiScene* scene);
    void process_mesh(aiMesh* mesh, const aiScene* scene);
    std::vector<TextureRef> load_material_textures(aiMaterial* mat, aiTextureType type, const char* type_name);
};

} // namespace Renderer
