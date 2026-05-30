#include "entity.hpp"

#include "scene.hpp"

#include "../app_data.hpp"

Entity::Entity(Scene* scene, entt::entity entity)
    : m_scene(scene)
    , m_entity(entity)
{
}

bool Entity::valid()
{
    if (m_entity == entt::null) {
        return false;
    }
    return m_scene->m_registry.valid(m_entity);
}

entt::entity Entity::get_id()
{
    return m_entity;
}

entt::registry& Entity::get_registry()
{
    return m_scene->m_registry;
}

Scene* Entity::get_scene()
{
    return m_scene;
}

void Entity::add_name(Entity entity, const char* name)
{
    entity.add_component<const char*>(name);
}

void Entity::add_model(Entity entity, const char* path, GlobalAppData* app_data)
{
    auto* model_cache = &app_data->m_model_cache;
    auto handle = model_cache->get_or_create(path, path, app_data);
    auto* model = model_cache->get(handle);

    if (model->get_mesh()->m_has_bones) {
        auto& data = entity.add_component<Renderer::AnimationData>();
        for (auto& animation : model->get_mesh()->m_animations) {
            data.data.emplace_back(animation.create_per_animation_data());
        }
    }

    entity.add_component<Renderer::Model*>(model);
    entity.get_scene()->m_models_instance_draw_cache_needs_update = true;
}

void Entity::add_physics_command(Entity entity, const PhysicsFn& create_body_function)
{
    util_assert(entity.has_component<Renderer::Model*>() == true, "Cannot add physics to an entity without a model");

    auto physics_info = create_body_function(entity.get_scene()->m_physics_system.get(), entity.get_component<Renderer::Model*>());

    entity.add_component<JPH::BodyID>(physics_info.first);
    entity.add_component<JPH::EMotionType>(physics_info.second);
    entity.get_scene()->m_physics_needs_optimize = true;
}

void Entity::add_pbr_directional_light(Entity entity, Renderer::Light::Pbr::Directional& info)
{
    entity.add_component<Renderer::Light::Pbr::Directional>(info);
    entity.get_scene()->m_shaders_need_update = true;
}

void Entity::add_pbr_directional_light_shadow(Entity entity)
{
    entity.add_component<Renderer::Light::Pbr::DirectionalShadow>().init();
    entity.get_scene()->m_shaders_need_update = true;
}

void Entity::add_pbr_point_light(Entity entity, Renderer::Light::Pbr::Point& info)
{
    entity.add_component<Renderer::Light::Pbr::Point>(info);
    entity.get_scene()->m_shaders_need_update = true;
}

void Entity::add_pbr_point_light_shadow(Entity entity)
{
    entity.add_component<Renderer::Light::Pbr::PointShadow>().init();
    entity.get_scene()->m_shaders_need_update = true;
}

void Entity::add_pbr_spot_light(Entity entity, Renderer::Light::Pbr::Spot& info)
{
    entity.add_component<Renderer::Light::Pbr::Spot>(info);
    entity.get_scene()->m_shaders_need_update = true;
}

void Entity::add_pbr_spot_light_shadow(Entity entity)
{
    entity.add_component<Renderer::Light::Pbr::SpotShadow>().init();
    entity.get_scene()->m_shaders_need_update = true;
}

void Entity::add_transform(Entity entity, const Transform& transform)
{
    entity.add_component<Transform>(transform);
}