#pragma once

#include "../physics/engine.hpp"
#include "renderer.hpp"

using PhysicsFn = std::function<std::pair<JPH::BodyID, JPH::EMotionType>(Physics::System* engine, Renderer::Model* model)>;

class EntityBuilder : public NoCopy {
    friend class Scene;

public:
    EntityBuilder& add_model_path(const char* path);
    EntityBuilder& add_physics_command(PhysicsFn& create_body_function);
    EntityBuilder& add_point_light(Renderer::Light::PointInfo& info);
    EntityBuilder& add_directional_light(Renderer::Light::DirectionalInfo& info);
    EntityBuilder& add_model_matrix(glm::mat4 model);

private:
    const char* m_model_path = nullptr;
    PhysicsFn m_create_body = nullptr;
    Renderer::Light::PointInfo* m_point_info = nullptr;
    Renderer::Light::DirectionalInfo* m_directional_info = nullptr;
    glm::mat4 m_model_matrix { 1.0 };
};