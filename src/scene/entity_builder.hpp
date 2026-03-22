#pragma once

#include "../physics/engine.hpp"
#include "renderer.hpp"

#include "transform.hpp"

using PhysicsFn = std::function<std::pair<JPH::BodyID, JPH::EMotionType>(Physics::System* engine, Renderer::Model* model)>;

class EntityBuilder : public NoCopy {
    friend class Scene;

public:
    EntityBuilder& add_name(const char* name);
    EntityBuilder& add_model_path(const char* path);
    EntityBuilder& add_physics_command(const PhysicsFn& create_body_function);
    EntityBuilder& add_pbr_directional_light(Renderer::Light::Pbr::Directional& info);
    EntityBuilder& add_pbr_directional_light_shadow();
    EntityBuilder& add_pbr_point_light(Renderer::Light::Pbr::Point& info);
    EntityBuilder& add_pbr_point_light_shadow();
    EntityBuilder& add_pbr_spot_light(Renderer::Light::Pbr::Spot& info);
    EntityBuilder& add_pbr_spot_light_shadow();

    EntityBuilder& add_transform(const Transform& transform);
    // EntityBuilder& add_model_matrix(const glm::mat4& model);

private:
    const char* m_model_path = nullptr;
    PhysicsFn m_create_body = nullptr;
    Renderer::Light::Pbr::Directional* m_pbr_directional = nullptr;
    bool m_pbr_directional_shadow = false;
    Renderer::Light::Pbr::Point* m_pbr_point = nullptr;
    bool m_pbr_point_shadow = false;
    Renderer::Light::Pbr::Spot* m_pbr_spot = nullptr;
    bool m_pbr_spot_shadow = false;
    Transform m_transform;
    // glm::mat4 m_model_matrix { 1.0 };
    const char* m_name = nullptr;
};
