#pragma once

#include "physics/engine.hpp"

#include "renderer.hpp"

using Entity = u32;
class EntityBuilder;

class Scene : public NoCopyNoMove {
public:
    void add_entity(EntityBuilder& entity);

private:
    void create_model(const char* path);
    void create_physics_body(std::function<JPH::BodyID(PhysicsEngine* engine)>& create_body_function);
    void create_point_light(Renderer::Light::PointInfo& info);
    void create_directional_light(Renderer::Light::DirectionalInfo& info);

    std::vector<Renderer::Light::Point> m_point_lights;
    std::vector<Renderer::Light::Directional> m_directional_lights;
    std::vector<Renderer::Model> m_models;
    std::vector<JPH::BodyID> m_bodies;

    std::vector<Entity> m_entities;
};

class EntityBuilder : public NoCopy {
public:
    EntityBuilder& add_model_path(const char* path);
    EntityBuilder& add_physics_command(std::function<JPH::BodyID(PhysicsEngine* engine, Renderer::Model* model)>& create_body_function);
    EntityBuilder& add_point_light(Renderer::Light::PointInfo& info);
    EntityBuilder& add_directional_light(Renderer::Light::DirectionalInfo& info);

private:
    char* m_model_path;
    std::function<JPH::BodyID(PhysicsEngine* engine, Renderer::Model* model)> m_create_body;
    Renderer::Light::PointInfo* m_point_info;
    Renderer::Light::DirectionalInfo* m_directional_info;
};