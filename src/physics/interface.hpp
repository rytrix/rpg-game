#pragma once

#include "../scene/entity.hpp"

namespace Physics {

enum class PhysicsType {
    Mesh,
    Shape,
};

struct PhysicsInfo {
    JPH::BodyID m_id;
    JPH::EMotionType m_motion_type {};

    PhysicsType m_type;

    JPH::Ref<JPH::Shape> m_shape;
    Entity m_entity;
};

PhysicsInfo create_static_body(Entity entity);
PhysicsInfo create_dynamic_body(Entity entity, JPH::Ref<JPH::Shape> shape);

} // namespace Physics 
