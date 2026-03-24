#pragma once

#include "../physics/engine.hpp"
#include "../utils/deltatime.hpp"

#include "renderer.hpp"

#include "entity_builder.hpp"

#include "../app_data.hpp"

class Scene : public NoCopyNoMove {
public:
    explicit Scene(GlobalAppData* app_data);
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
    void compile_pbr_shaders(const std::string& empty_defines, const std::string& bone_defines);

    Utils::DeltaTime m_clock;

    GlobalAppData* m_app_data;
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

    struct ModelMatrix {
        Renderer::Model* m_model;
        std::vector<glm::mat4> m_model_matrices;

        ModelMatrix(Renderer::Model* model, glm::mat4 matrix)
            : m_model(model)
            , m_model_matrices({ matrix })
        {
        }
    };
    Handle random_sampling_texture;

    std::vector<ModelMatrix> m_models_instance_draw_cache;
    bool m_models_instance_draw_cache_needs_update = false;

    bool m_physics_needs_optimize = false;
    std::unique_ptr<Physics::System> m_physics_system = nullptr;
};
