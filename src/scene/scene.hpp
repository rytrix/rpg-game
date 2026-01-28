#pragma once

#include "../physics/engine.hpp"

#include "renderer.hpp"

#include "entity_builder.hpp"

class Scene : public NoCopyNoMove {
public:
    Scene();
    ~Scene() = default;

    void add_entity(EntityBuilder& entity);

    void compile_shaders();

private:
    Renderer::ShaderProgram m_gpass_shader;
    Renderer::ShaderProgram m_lpass_shader;

    // TODO: do I make these global? they never change.
    Renderer::ShaderProgram m_shadowmap_shader;
    Renderer::ShaderProgram m_shadowmap_cubemap_shader;

    entt::registry m_registry;
    std::unique_ptr<Physics::System> m_physics_system = nullptr;
    // std::vector<Entity> m_entities;
};
