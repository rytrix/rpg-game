#pragma once

#include "scene.hpp"

#include "transform.hpp"

using PhysicsFn = std::function<std::pair<JPH::BodyID, JPH::EMotionType>(Physics::System* engine, Renderer::Model* model)>;

class Entity {
public:
    Entity() = default;
    Entity(Scene* scene, entt::entity entity);

    template <typename T, typename... Args>
    T& add_component(Args&&... args);

    template <typename T>
    void remove_component();

    template <typename T>
    T& get_component();

    template <typename T>
    bool has_component();

    entt::entity get_id() { return m_entity; }
    entt::registry& get_registry() { return m_scene->m_registry; }
    Scene* get_scene() { return m_scene; }

    // Helper functions
    static void add_name(Entity entity, const char* name);
    static void add_model(Entity entity, const char* path, GlobalAppData* app_data);
    static void add_physics_command(Entity entity, const PhysicsFn& create_body_function);
    static void add_pbr_directional_light(Entity entity, Renderer::Light::Pbr::Directional& info);
    static void add_pbr_directional_light_shadow(Entity entity);
    static void add_pbr_point_light(Entity entity, Renderer::Light::Pbr::Point& info);
    static void add_pbr_point_light_shadow(Entity entity);
    static void add_pbr_spot_light(Entity entity, Renderer::Light::Pbr::Spot& info);
    static void add_pbr_spot_light_shadow(Entity entity);
    static void add_transform(Entity entity, const Transform& transform);

private:
    Scene* m_scene = nullptr;
    entt::entity m_entity = entt::null;
};

template <typename T, typename... Args>
T& Entity::add_component(Args&&... args)
{
    util_assert(has_component<T>() == false, std::format("Entity already has component \"{}\"", typeid(T).name()));
    return m_scene->m_registry.emplace<T>(m_entity, std::forward<Args>(args)...);
}

template <typename T>
void Entity::remove_component()
{
    m_scene->m_registry.remove<T>(m_entity);
}

template <typename T>
T& Entity::get_component()
{
    return m_scene->m_registry.get<T>(m_entity);
}

template <typename T>
bool Entity::has_component()
{
    return m_scene->m_registry.all_of<T>(m_entity);
}