#pragma once

#include "mesh.hpp"
#include "shader.hpp"
#include "texture.hpp"

#include <assimp/scene.h>

namespace Renderer {

class Model : public NoCopyNoMove {
public:
    Model() = default;
    explicit Model(const char* path);
    ~Model();

    void init(const char* path);

    void draw_untextured(ShaderProgram& shader);
    void draw(ShaderProgram& shader);

    const std::deque<Mesh>* get_meshes();
    glm::mat4& get_model_matrix();

private:
    bool initialized = false;

    std::deque<TextureStorage> m_textures;
    std::deque<Mesh> m_meshes;
    std::string m_directory;
    glm::mat4 u_model { 1.0 };

    void process_node(aiNode* node, const aiScene* scene);
    void process_mesh(aiMesh* mesh, const aiScene* scene);
    std::vector<TextureRef> load_material_textures(aiMaterial* mat, aiTextureType type, const char* type_name);
};

} // namespace Renderer
