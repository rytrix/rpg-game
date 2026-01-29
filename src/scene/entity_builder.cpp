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

EntityBuilder& EntityBuilder::add_point_light(Renderer::Light::PointInfo& info)
{
    m_point_info = &info;
    return *this;
}

EntityBuilder& EntityBuilder::add_directional_light(Renderer::Light::DirectionalInfo& info)
{
    m_directional_info = &info;
    return *this;
}

EntityBuilder& EntityBuilder::add_model_matrix(glm::mat4 model)
{
    m_model_matrix = model;
    return *this;
}