#pragma once

#include "assimp/Importer.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "texture.hpp"

#include "../utils/cache.hpp"
#include "../utils/deltatime.hpp"

#include <assimp/scene.h>

namespace Renderer {

class Model : public NoCopyNoMove {
public:
    Model() = default;
    explicit Model(const char* path);
    ~Model();

    void init(const char* path);

    void update(std::span<glm::mat4> models, double animation_time);

    void draw_untextured(ShaderProgram& shader);
    void draw(ShaderProgram& shader);

    const Mesh* get_mesh();
    bool has_bones() const;

    // Doesn't need to be called it will lazy load (or do before model loading if it is multi-threaded)
    static void init_placeholder_textures();
    // Call this before the opengl context is killed (or don't the os will clean it up)
    static void destroy_placeholder_textures();

private:
    bool initialized = false;

    Utils::Cache<std::string, Texture> m_texture_cache;

    Assimp::Importer m_importer;
    Mesh m_mesh;
    std::string m_directory;

    Utils::DeltaTime m_timer;

    void process_node(aiNode* node, const aiScene* scene);
    void process_mesh(aiMesh* mesh, const aiScene* scene);
    Texture* load_material_texture(const aiMaterial* mat, const aiTextureType type, const aiScene* scene);

    static Texture* get_placeholder_texture_albedo();
    static Texture* get_placeholder_texture_normal();
    static Texture* get_placeholder_texture_metallic();
};

} // namespace Renderer
