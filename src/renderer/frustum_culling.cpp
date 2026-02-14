#include "frustum_culling.hpp"

namespace Renderer {

Plane::Plane(const glm::vec3& p1, const glm::vec3& norm)
    : normal(glm::normalize(norm))
    , distance(glm::dot(normal, p1))
{
}

float Plane::get_signed_distance_to_plane(const glm::vec3& point) const
{
    return glm::dot(normal, point) - distance;
}

glm::vec3 Plane::get_normal() const
{
    return normal;
}

float Plane::get_distance() const
{
    return distance;
}

Frustum::Frustum(const Camera& cam)
{
    init(cam);
}

void Frustum::init(const Camera& cam)
{
    const glm::vec3 front = cam.get_front();
    const glm::vec3 pos = cam.get_pos();
    const glm::vec3 right = cam.get_right();
    const glm::vec3 up = cam.get_up();
    const float z_far = cam.get_far();
    const float z_near = cam.get_near();
    const float aspect = cam.get_aspect();
    const float fov_y = cam.get_fov();

    const float halfvside = z_far * tanf(fov_y * .5f);
    const float halfhside = halfvside * aspect;
    const glm::vec3 front_mult_far = z_far * front;

    near_face = { pos + z_near * front, front };
    far_face = { pos + front_mult_far, -front };
    right_face = { pos,
        glm::cross(front_mult_far - right * halfhside, up) };
    left_face = { pos,
        glm::cross(up, front_mult_far + right * halfhside) };
    top_face = { pos,
        glm::cross(right, front_mult_far - up * halfvside) };
    bottom_face = { pos,
        glm::cross(front_mult_far + up * halfvside, right) };
}

} // namespace Renderer

// struct AABB {
//     inline AABB() = default;
//     inline AABB(const glm::vec3& min, const glm::vec3& max);

//     inline void init(const glm::vec3& min, const glm::vec3& max);

//     inline std::array<glm::vec3, 8> getVerticies() const;
//     inline bool isOnOrForwardPlane(const Plane& plane) const;
//     inline bool isOnFrustum(const Frustum& camFrustum) const;

// private:
//     glm::vec3 center { 0.f, 0.f, 0.f };
//     glm::vec3 extents { 0.f, 0.f, 0.f };
// };

// inline AABB::AABB(const glm::vec3& min, const glm::vec3& max)
// {
//     init(min, max);
// }

// inline void AABB::init(const glm::vec3& min, const glm::vec3& max)
// {
//     center = { (max + min) * 0.5f };
//     extents = { max.x - center.x, max.y - center.y, max.z - center.z };
// }

// inline std::array<glm::vec3, 8> AABB::getVerticies() const
// {
//     std::array<glm::vec3, 8> verticies;
//     verticies[0] = { center.x - extents.x, center.y - extents.y, center.z - extents.z };
//     verticies[1] = { center.x + extents.x, center.y - extents.y, center.z - extents.z };
//     verticies[2] = { center.x - extents.x, center.y + extents.y, center.z - extents.z };
//     verticies[3] = { center.x + extents.x, center.y + extents.y, center.z - extents.z };
//     verticies[4] = { center.x - extents.x, center.y - extents.y, center.z + extents.z };
//     verticies[5] = { center.x + extents.x, center.y - extents.y, center.z + extents.z };
//     verticies[6] = { center.x - extents.x, center.y + extents.y, center.z + extents.z };
//     verticies[7] = { center.x + extents.x, center.y + extents.y, center.z + extents.z };
//     return verticies;
// }

// inline bool AABB::isOnOrForwardPlane(const Plane& plane) const
// {
//     // Compute the projection interval radius of b onto L(t) = b.c + t * p.n
//     const glm::vec3 normal = plane.getNormal();
//     const float r = extents.x * std::abs(normal.x) + extents.y * std::abs(normal.y) + extents.z * std::abs(normal.z);

//     return -r <= plane.getSignedDistanceToPlane(center);
// }

// inline bool AABB::isOnFrustum(const Frustum& camFrustum) const
// {
//     return (isOnOrForwardPlane(camFrustum.leftFace) && isOnOrForwardPlane(camFrustum.rightFace) && isOnOrForwardPlane(camFrustum.topFace) && isOnOrForwardPlane(camFrustum.bottomFace) && isOnOrForwardPlane(camFrustum.nearFace) && isOnOrForwardPlane(camFrustum.farFace));
// };