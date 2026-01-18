#pragma once

static constexpr JPH::Float3 vec3_to_float3(glm::vec3 vec)
{
    return { vec.x, vec.y, vec.z };
}

static constexpr JPH::Vec3 vec3_to_vec3(glm::vec3 vec)
{
    return { vec.x, vec.y, vec.z };
}

static constexpr glm::mat4 mat4_to_mat4(JPH::Mat44 mat4)
{
    glm::mat4 out;
    for (u32 row = 0; row < 4; ++row) {
        for (u32 col = 0; col < 4; ++col) {
            out[col][row] = mat4(row, col); // this is the line causing the error.
        }
    }

    return out;
}
