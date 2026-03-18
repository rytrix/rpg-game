#pragma once

#include "../physics/engine.hpp"
#include "../utils/cache.hpp"
#include "../utils/deltatime.hpp"

#include "resource_manager.hpp"

#include "renderer.hpp"

#include "entity_builder.hpp"

class Scene : public NoCopyNoMove {
public:
    explicit Scene(Renderer::Window& window, Renderer::Camera& camera);
    ~Scene();

    void add_entity(const EntityBuilder& entity);

    // Call update after adding an entity (or maybe I do that internally)
    void update();
    void optimize();

    void physics();
    void draw();

    void draw_debug_imgui();

    Renderer::Camera& get_camera();
    const Utils::DeltaTime& get_clock();

private:
    void compile_shaders();
    void compile_pbr_shaders(const std::string& bone_defines);

    Utils::DeltaTime m_clock;

    Renderer::Window& m_window;
    Renderer::Camera& m_camera;
    float m_camera_speed = 5.0F;

    bool m_shaders_need_update = true;

    // TODO: do I make these global? they never change.
    Renderer::ShaderProgram m_shader;
    Renderer::ShaderProgram m_shader_bones;
    Renderer::ShaderProgram m_shadowmap_shader;
    Renderer::ShaderProgram m_shadowmap_shader_bones;
    Renderer::ShaderProgram m_shadowmap_cubemap_shader;
    Renderer::ShaderProgram m_shadowmap_cubemap_shader_bones;

    entt::registry m_registry;
    // Utils::Cache<const char*, Renderer::Model> m_model_cache;
    ResourceManager<const char*, Renderer::Model, 100> m_model_cache;

    struct ModelMatrix {
        Renderer::Model* m_model;
        std::vector<glm::mat4> m_model_matrices;

        ModelMatrix(Renderer::Model* model, glm::mat4 matrix)
            : m_model(model)
            , m_model_matrices({ matrix })
        {
        }
    };
    std::vector<ModelMatrix> m_models_instance_draw_cache;
    bool m_models_instance_draw_cache_needs_update = false;

    bool m_physics_needs_optimize = false;
    std::unique_ptr<Physics::System> m_physics_system = nullptr;
};
