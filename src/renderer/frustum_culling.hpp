#pragma once

#include "camera.hpp"

namespace Renderer {

struct Plane {
    Plane() = default;

    Plane(const glm::vec3& p1, const glm::vec3& norm);

    [[nodiscard]] float get_signed_distance_to_plane(const glm::vec3& point) const;

    [[nodiscard]] glm::vec3 get_normal() const;
    [[nodiscard]] float get_distance() const;

private:
    glm::vec3 normal { 0.0F, 1.0F, 0.0F };
    float distance {};
};

struct Frustum {
    Frustum() = default;
    explicit Frustum(const Camera& cam);
    void init(const Camera& cam);

    Plane top_face;
    Plane bottom_face;

    Plane right_face;
    Plane left_face;

    Plane far_face;
    Plane near_face;
};

} // namespace Renderer
