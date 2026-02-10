#include "entity_builder.hpp"

EntityBuilder& EntityBuilder::add_name(const char* name)
{
    m_name = name;
    return *this;
}

EntityBuilder& EntityBuilder::add_model_path(const char* path)
{
    m_model_path = path;
    return *this;
}

EntityBuilder& EntityBuilder::add_physics_command(const PhysicsFn& create_body_function)
{
    m_create_body = create_body_function;
    return *this;
}

EntityBuilder& EntityBuilder::add_phong_point_light(Renderer::Light::Phong::PointInfo& info)
{
    m_phong_point_info = &info;
    return *this;
}

EntityBuilder& EntityBuilder::add_phong_directional_light(Renderer::Light::Phong::DirectionalInfo& info)
{
    m_phong_directional_info = &info;
    return *this;
}

EntityBuilder& EntityBuilder::add_pbr_point_light(Renderer::Light::Pbr::Point& info)
{
    m_pbr_point = &info;
    return *this;
}

EntityBuilder& EntityBuilder::add_pbr_directional_light(Renderer::Light::Pbr::Directional& info)
{
    m_pbr_directional = &info;
    return *this;
}

EntityBuilder& EntityBuilder::add_pbr_spot_light(Renderer::Light::Pbr::Spot& info)
{
    m_pbr_spot = &info;
    return *this;
}

EntityBuilder& EntityBuilder::add_model_matrix(glm::mat4 model)
{
    m_model_matrix = model;
    return *this;
}