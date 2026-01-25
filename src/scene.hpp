#pragma once

#include "physics/engine.hpp"

#include "renderer.hpp"

using Entity = u32;
class EntityBuilder;

using PhysicsFn = std::function<JPH::BodyID(Physics::System* engine, Renderer::Model* model)>;

class Scene : public NoCopyNoMove {
public:
    void add_entity(EntityBuilder& entity);

private:
    void create_model(const char* path);
    void create_physics_body(PhysicsFn& create_body_function);
    void create_point_light(Renderer::Light::PointInfo& info);
    void create_directional_light(Renderer::Light::DirectionalInfo& info);

    void compile_shader();

    Renderer::ShaderProgram m_lpass_shader;

    // TODO
    // Make removal of elements efficient
    std::vector<Renderer::Light::Point> m_point_lights;
    std::vector<Renderer::Light::Directional> m_directional_lights;
    std::vector<Renderer::Model> m_models;
    std::vector<JPH::BodyID> m_bodies;

    struct Mapping {
        u32 point;
        u32 directional;
        u32 model;
        u32 body;
    };

    std::unordered_map<Entity, Mapping> m_mappings;

    std::vector<Entity> m_entities;
};

class EntityBuilder : public NoCopy {
public:
    EntityBuilder& add_model_path(const char* path);
    EntityBuilder& add_physics_command(PhysicsFn& create_body_function);
    EntityBuilder& add_point_light(Renderer::Light::PointInfo& info);
    EntityBuilder& add_directional_light(Renderer::Light::DirectionalInfo& info);

private:
    char* m_model_path;
    PhysicsFn m_create_body;
    Renderer::Light::PointInfo* m_point_info;
    Renderer::Light::DirectionalInfo* m_directional_info;
};