#pragma once

namespace Utils {

struct Line {
    glm::vec3 position;
    glm::vec3 direction;
    f32 length;
    f32 thickness;
};

} // namespace Utils