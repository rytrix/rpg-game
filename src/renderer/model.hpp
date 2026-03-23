#pragma once

#include "mesh.hpp"
#include "shader.hpp"
#include "texture.hpp"

#include "../scene/resource_manager2.hpp"
#include "../utils/deltatime.hpp"

#include "../app_data.hpp"

#include <assimp/scene.h>

namespace Renderer {

class Model : public NoCopyNoMove {
public:
    Model() = default;
    Model(const char* path, GlobalAppData* app_data);
    ~Model();

    void init(const char* path, GlobalAppData* app_data);

    void update(std::span<glm::mat4> models, float animation_time);

    void draw_untextured(ShaderProgram& shader);
    void draw(ShaderProgram& shader);

    const Mesh* get_mesh();
    bool has_bones() const;

    std::deque<Animation>& get_animations();
    u32 get_current_animation() const;
    void set_animation(u32 value);

private:
    bool initialized = false;

    GlobalAppData* m_app_data = nullptr;

    Mesh m_mesh;
    std::string m_directory;

    std::deque<Animation> m_animations;
    u32 m_current_animation = 0;

    void process_node(aiNode* node, const aiScene* scene);
    void process_mesh(aiMesh* mesh, const aiScene* scene);
    Handle load_material_texture(const aiMaterial* mat, const aiTextureType type, const aiScene* scene);
};

} // namespace Renderer
