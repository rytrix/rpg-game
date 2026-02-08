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

    void draw_untextured(ShaderProgram& shader, const std::vector<glm::mat4>& model);
    void draw(ShaderProgram& shader, const std::vector<glm::mat4>& model);

    const Mesh* get_mesh();

    // Doesn't need to be called it will lazy load (or do before model loading if it is multi-threaded)
    static void init_placeholder_textures();
    // Call this before the opengl context is killed (or don't the os will clean it up)
    static void destroy_placeholder_textures();
private:
    bool initialized = false;

    Utils::Cache<std::string, Texture> m_texture_cache;
    Mesh m_mesh;
    std::string m_directory;

    void process_node(aiNode* node, const aiScene* scene);
    void process_mesh(aiMesh* mesh, const aiScene* scene);
    Texture* load_material_texture(aiMaterial* mat, aiTextureType type);

    static Texture* get_placeholder_texture_albedo();
    static Texture* get_placeholder_texture_normal();
    static Texture* get_placeholder_texture_metallic();
};

} // namespace Renderer
