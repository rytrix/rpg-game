#pragma once

#include <assimp/matrix4x4.h>
#include <assimp/quaternion.h>
#include <assimp/vector3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

static constexpr glm::mat4 mat4_to_mat4(const aiMatrix4x4& from)
{
    return glm::transpose(glm::make_mat4(&from.a1));
}

static constexpr glm::vec3 vec3_to_vec3(const aiVector3D& vec)
{
    return { vec.x, vec.y, vec.z };
}

static constexpr glm::quat quat_to_quat(const aiQuaternion& pOrientation)
{
    return { pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z };
}