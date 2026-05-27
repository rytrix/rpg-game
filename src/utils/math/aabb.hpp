#pragma once

namespace Utils {

struct AABB {
    glm::vec3 min;
    glm::vec3 max;

    [[nodiscard]] AABB transform(const glm::mat4& transform) const;
    [[nodiscard]] bool intersection(const glm::vec3& position) const;

private:
    [[nodiscard]] AABB transform_naive(const glm::mat4& transform);
    [[nodiscard]] AABB transform_fast(const glm::mat4& transform) const;
};

} // namespace Utils
