#pragma once

struct AppData;

#include "../app_data.hpp"

#include "mesh.hpp"
#include "shader.hpp"
#include "texture.hpp"

#include "../scene/resource_manager.hpp"
#include "../utils/deltatime.hpp"

#include <assimp/scene.h>

namespace Renderer {

class Model : public NoCopyNoMove {
public:
    Model() = default;
    Model(const char* path, GlobalAppData* app_data);
    ~Model();

    void init(const char* path, GlobalAppData* app_data);

    void update(std::span<glm::mat4> models, std::span<PerAnimationData*> animation_data);

    void draw_untextured(ShaderProgram& shader);
    void draw(ShaderProgram& shader);

    const Mesh* get_mesh();
    bool has_bones() const;

    std::deque<Animation>& get_animations();

private:
    bool initialized = false;

    GlobalAppData* m_app_data = nullptr;

    std::string m_directory;

    Mesh m_mesh;
    std::deque<Animation> m_animations;

    void setup_mesh(const aiScene* scene);
    void setup_animations(const aiScene* scene);

    void process_node(aiNode* node, const aiScene* scene);
    void process_mesh(aiMesh* mesh, const aiScene* scene);
    Handle load_material_texture(const aiMaterial* mat, const aiTextureType type, const aiScene* scene);
};

} // namespace Renderer
