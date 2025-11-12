#pragma once

#include "shader.hpp"
#include "texture.hpp"
#include "mesh.hpp"

#include <assimp/scene.h>

namespace Renderer {

class Model {
public:
    explicit Model(const char* path);
    ~Model() = default;

    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;
    Model(Model&&) = default;
    Model& operator=(Model&&) = default;

    void draw(ShaderProgram& shader);

private:
    std::vector<TextureStorage> m_textures;
    std::vector<Mesh> m_meshes;
    std::string m_directory;

    void process_node(aiNode* node, const aiScene* scene);
    Mesh process_mesh(aiMesh* mesh, const aiScene* scene);
    std::vector<TextureRef> load_material_textures(aiMaterial* mat, aiTextureType type, const char* type_name);
};

} // namespace Renderer
