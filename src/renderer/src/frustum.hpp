#pragma once

#include "camera.hpp"

#include <array>

namespace Renderer {

struct Plane {
    inline Plane() = default;

    inline Plane(const glm::vec3& p1, const glm::vec3& norm);

    inline float getSignedDistanceToPlane(const glm::vec3& point) const;

    inline glm::vec3 getNormal() const;
    inline float getDistance() const;

private:
    glm::vec3 normal { 0.f, 1.f, 0.f };
    float distance {};
};

inline Plane::Plane(const glm::vec3& p1, const glm::vec3& norm)
    : normal(glm::normalize(norm))
    , distance(glm::dot(normal, p1))
{
}

inline float Plane::getSignedDistanceToPlane(const glm::vec3& point) const
{
    return glm::dot(normal, point) - distance;
}

inline glm::vec3 Plane::getNormal() const
{
    return normal;
}
inline float Plane::getDistance() const
{
    return distance;
}

struct Frustum {
    friend struct AABB;
    inline Frustum(const Camera& cam);

    inline void init(const Camera& cam);

private:
    Plane topFace;
    Plane bottomFace;

    Plane rightFace;
    Plane leftFace;

    Plane farFace;
    Plane nearFace;
};

inline Frustum::Frustum(const Camera& cam)
{
	init(cam);
}

inline void Frustum::init(const Camera& cam)
{
    const glm::vec3 front = cam.getFront();
    const glm::vec3 pos = cam.getPos();
    const glm::vec3 right = cam.getRight();
    const glm::vec3 up = cam.getUp();
    const float zFar = cam.getFar();
    const float zNear = cam.getNear();
    const float aspect = cam.getAspect();
    const float fovY = cam.getFov();

    const float halfVSide = zFar * tanf(fovY * .5f);
    const float halfHSide = halfVSide * aspect;
    const glm::vec3 frontMultFar = zFar * front;

    nearFace = { pos + zNear * front, front };
    farFace = { pos + frontMultFar, -front };
    rightFace = { pos,
        glm::cross(frontMultFar - right * halfHSide, up) };
    leftFace = { pos,
        glm::cross(up, frontMultFar + right * halfHSide) };
    topFace = { pos,
        glm::cross(right, frontMultFar - up * halfVSide) };
    bottomFace = { pos,
        glm::cross(frontMultFar + up * halfVSide, right) };
}

struct AABB {
    inline AABB() = default;
    inline AABB(const glm::vec3& min, const glm::vec3& max);

    inline void init(const glm::vec3& min, const glm::vec3& max);

    inline std::array<glm::vec3, 8> getVerticies() const;
    inline bool isOnOrForwardPlane(const Plane& plane) const;
    inline bool isOnFrustum(const Frustum& camFrustum) const;

private:
    glm::vec3 center { 0.f, 0.f, 0.f };
    glm::vec3 extents { 0.f, 0.f, 0.f };
};

inline AABB::AABB(const glm::vec3& min, const glm::vec3& max)
{
    init(min, max);
}

inline void AABB::init(const glm::vec3& min, const glm::vec3& max)
{
    center = { (max + min) * 0.5f };
    extents = { max.x - center.x, max.y - center.y, max.z - center.z };
}

inline std::array<glm::vec3, 8> AABB::getVerticies() const
{
    std::array<glm::vec3, 8> verticies;
    verticies[0] = { center.x - extents.x, center.y - extents.y, center.z - extents.z };
    verticies[1] = { center.x + extents.x, center.y - extents.y, center.z - extents.z };
    verticies[2] = { center.x - extents.x, center.y + extents.y, center.z - extents.z };
    verticies[3] = { center.x + extents.x, center.y + extents.y, center.z - extents.z };
    verticies[4] = { center.x - extents.x, center.y - extents.y, center.z + extents.z };
    verticies[5] = { center.x + extents.x, center.y - extents.y, center.z + extents.z };
    verticies[6] = { center.x - extents.x, center.y + extents.y, center.z + extents.z };
    verticies[7] = { center.x + extents.x, center.y + extents.y, center.z + extents.z };
    return verticies;
}

inline bool AABB::isOnOrForwardPlane(const Plane& plane) const
{
    // Compute the projection interval radius of b onto L(t) = b.c + t * p.n
    const glm::vec3 normal = plane.getNormal();
    const float r = extents.x * std::abs(normal.x) + extents.y * std::abs(normal.y) + extents.z * std::abs(normal.z);

    return -r <= plane.getSignedDistanceToPlane(center);
}

inline bool AABB::isOnFrustum(const Frustum& camFrustum) const
{
    return (isOnOrForwardPlane(camFrustum.leftFace) && isOnOrForwardPlane(camFrustum.rightFace) && isOnOrForwardPlane(camFrustum.topFace) && isOnOrForwardPlane(camFrustum.bottomFace) && isOnOrForwardPlane(camFrustum.nearFace) && isOnOrForwardPlane(camFrustum.farFace));
};

} // namespace Renderer
