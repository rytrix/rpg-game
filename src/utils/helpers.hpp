#pragma once

#include "assimp/matrix4x4.h"

static constexpr glm::mat4 mat4_to_mat4(aiMatrix4x4 mat4)
{
    glm::mat4 out;
    for (u32 row = 0; row < 4; row++) {
        for (u32 col = 0; col < 4; col++) {
            out[static_cast<int>(col)][static_cast<int>(row)] = mat4[row][col];
        }
    }

    return out;
}