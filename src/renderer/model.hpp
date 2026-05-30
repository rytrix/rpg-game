#pragma once

class GlobalAppData;

#include "mesh.hpp"
#include "shader.hpp"

#include "../scene/resource_manager.hpp"

#include <assimp/scene.h>

namespace Renderer {

class Model : public NoCopyNoMove {
public:
    Model() = default;
    Model(const char* path, GlobalAppData* app_data);
    ~Model();

    void init(const char* path, GlobalAppData* app_data);

    void update(std::span<glm::mat4> models, std::span<AnimationData*> animation_data);

    void draw_untextured(Shader& shader);
    void draw(Shader& shader);

    Mesh* get_mesh();

private:
    bool initialized = false;

    GlobalAppData* m_app_data = nullptr;

    std::string m_directory;

    Mesh m_mesh;

    void setup_mesh(const aiScene* scene);
    void setup_animations(const aiScene* scene);

    void process_node(aiNode* node, const aiScene* scene);
    void process_mesh(aiMesh* mesh, const aiScene* scene);
    Handle load_material_texture(const aiMaterial* mat, const aiTextureType type, const aiScene* scene);
};

} // namespace Renderer
