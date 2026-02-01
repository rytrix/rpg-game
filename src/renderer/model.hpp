#pragma once

#include "mesh.hpp"
#include "shader.hpp"
#include "texture.hpp"

#include "../utils/cache.hpp"

#include <assimp/scene.h>

namespace Renderer {

class Model : public NoCopyNoMove {
public:
    Model() = default;
    explicit Model(const char* path);
    ~Model();

    void init(const char* path);

    void draw_untextured(ShaderProgram& shader, glm::mat4 model) const;
    void draw(ShaderProgram& shader, glm::mat4 model) const;

    // const std::deque<Mesh>* get_meshes();
    const Mesh* get_mesh();

private:
    bool initialized = false;

    Utils::Cache<std::string, Texture> m_texture_cache;
    // std::deque<Mesh> m_meshes;
    Mesh m_mesh;
    std::string m_directory;

    void process_node(aiNode* node, const aiScene* scene);
    void process_mesh(aiMesh* mesh, const aiScene* scene);
    std::vector<TextureRef> load_material_textures(aiMaterial* mat, aiTextureType type);
};

} // namespace Renderer
