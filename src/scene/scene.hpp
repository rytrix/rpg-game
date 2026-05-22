#pragma once

#include "../physics/engine.hpp"
#include "../utils/deltatime.hpp"

#include "renderer.hpp"

#include "../app_data.hpp"

class Entity;

class Scene : public NoCopyNoMove {
    friend class Entity;
    friend class EntityHelper;

public:
    explicit Scene(GlobalAppData* app_data);
    ~Scene();

    Entity create_entity();
    // void add_entity(const EntityBuilder& entity);

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
    Renderer::Shader m_shader;
    Renderer::Shader m_shader_bones;
    Renderer::Shader m_shadowmap_shader;
    Renderer::Shader m_shadowmap_shader_bones;
    Renderer::Shader m_shadowmap_cubemap_shader;
    Renderer::Shader m_shadowmap_cubemap_shader_bones;

    entt::registry m_registry;

    struct ModelMatrix {
        Renderer::Model* m_model;
        std::vector<glm::mat4> m_model_matrices;
        std::vector<Renderer::AnimationData*> m_animation_data;

        ModelMatrix(Renderer::Model* model, glm::mat4 matrix)
            : m_model(model)
            , m_model_matrices({ matrix })
        {
        }
    };
    Renderer::RandomSamplingTexture m_random_sampling_texture;

    std::vector<ModelMatrix> m_models_instance_draw_cache;
    bool m_models_instance_draw_cache_needs_update = false;

    bool m_physics_needs_optimize = false;
    std::unique_ptr<Physics::System> m_physics_system = nullptr;
};

#include "entity.hpp"
