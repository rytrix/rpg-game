#pragma once

static constexpr JPH::Float3 vec3_to_float3(glm::vec3 vec)
{
    return { vec.x, vec.y, vec.z };
}

static constexpr JPH::Vec3 vec3_to_vec3(glm::vec3 vec)
{
    return { vec.x, vec.y, vec.z };
}
