#pragma once

#include "aabb.hpp"
#include "line.hpp"
#include "plane.hpp"
#include "ring.hpp"

struct GlobalAppData;

namespace Utils {

struct Ray {
    glm::vec3 position;
    glm::vec3 direction;

    Ray(glm::vec3 in_position, glm::vec3 in_direction);

    glm::vec3 get_inverse();

private:
    bool inverse_needs_update = true;
    glm::vec3 inv_direction {};
};

[[nodiscard]] Utils::Ray ray_from_mouse(GlobalAppData* data);
[[nodiscard]] Utils::Ray ray_from_center(GlobalAppData* data);

[[nodiscard]] std::optional<glm::vec3> intersect_ray_ring(const Ray& ray, const Ring& ring);
[[nodiscard]] std::optional<glm::vec3> intersect_ray_plane(const Ray& ray, const Plane& plane);
[[nodiscard]] std::optional<glm::vec3> intersect_ray_line(const Ray& ray, const Line& line);
[[nodiscard]] std::optional<glm::vec3> intersect_ray_line_closest(const Ray& ray, const Line& line);

[[nodiscard]] bool intersect_ray_aabb(Ray& ray, const AABB& aabb);
[[nodiscard]] std::optional<glm::vec3> intersect_ray_aabb_hit(Ray& ray, const AABB& aabb);

} // namespace Utils
