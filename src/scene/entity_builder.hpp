#pragma once

#include "../physics/engine.hpp"
#include "renderer.hpp"

using PhysicsFn = std::function<std::pair<JPH::BodyID, JPH::EMotionType>(Physics::System* engine, Renderer::Model* model)>;

class EntityBuilder : public NoCopy {
    friend class Scene;

public:
    EntityBuilder& add_name(const char* name);
    EntityBuilder& add_model_path(const char* path);
    EntityBuilder& add_physics_command(const PhysicsFn& create_body_function);
    EntityBuilder& add_phong_point_light(Renderer::Light::Phong::PointInfo& info);
    EntityBuilder& add_phong_directional_light(Renderer::Light::Phong::DirectionalInfo& info);
    EntityBuilder& add_pbr_point_light(Renderer::Light::Pbr::Point& info);
    EntityBuilder& add_pbr_directional_light(Renderer::Light::Pbr::Directional& info);
    EntityBuilder& add_pbr_spot_light(Renderer::Light::Pbr::Spot& info);
    EntityBuilder& add_model_matrix(glm::mat4 model);

private:
    const char* m_model_path = nullptr;
    PhysicsFn m_create_body = nullptr;
    Renderer::Light::Phong::PointInfo* m_phong_point_info = nullptr;
    Renderer::Light::Phong::DirectionalInfo* m_phong_directional_info = nullptr;
    Renderer::Light::Pbr::Point* m_pbr_point = nullptr;
    Renderer::Light::Pbr::Directional* m_pbr_directional = nullptr;
    Renderer::Light::Pbr::Spot* m_pbr_spot = nullptr;
    glm::mat4 m_model_matrix { 1.0 };
    const char* m_name = nullptr;
};