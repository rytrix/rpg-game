#pragma once

#include "../physics/engine.hpp"
#include "../utils/deltatime.hpp"

#include "renderer.hpp"

#include "../app_data.hpp"

class Entity;

class Scene : public NoCopyNoMove {
    friend class Entity;

public:
    explicit Scene(GlobalAppData* app_data);
    ~Scene();

    Entity create_entity();
    void remove_entity(Entity entity);

    // Crashes if the scene already has this component
    template <typename T, typename... Args>
    T& add_component(Args&&... args);

    template <typename T>
    void remove_component();

    template <typename T>
    T& get_component();

    template <typename T>
    bool has_component();

    // Call update after adding an entity (or maybe I do that internally)
    void update();

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

template <typename T, typename... Args>
T& Scene::add_component(Args&&... args)
{
    // if (has_component<T>()) {
    //     LOG_ERROR(std::format("Scene already has component \"{}\"", typeid(T).name()));
    //     return;
    // }
    util_assert(has_component<T>() == false, std::format("Scene already has component \"{}\"", typeid(T).name()));
    return m_registry.ctx().emplace<T>(std::forward<Args>(args)...);
}

template <typename T>
void Scene::remove_component()
{
    m_registry.ctx().erase<T>();
}

template <typename T>
T& Scene::get_component()
{
    return m_registry.ctx().get<T>();
}

template <typename T>
bool Scene::has_component()
{
    return m_registry.ctx().contains<T>();
}

#include "entity.hpp"
