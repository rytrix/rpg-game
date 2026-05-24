#pragma once

namespace Renderer {

struct Ray {
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 inv_direction;

    Ray(glm::vec3 in_position, glm::vec3 in_direction)
        : position(in_position)
        , direction(in_direction)
    {
        inv_direction = 1.0F / in_direction;
    }
};

struct Plane {
    glm::vec3 normal;
};

struct Ring {
    glm::vec3 position;
    glm::vec3 normal;
    f32 radius;
    f32 thickness;
};

constexpr std::optional<float> intersect_ray_ring(const Ray& ray, const Ring& ring)
{
    float denom = glm::dot(ray.direction, ring.normal);

    if (glm::abs(denom) < 1e-6F) {
        return std::nullopt; // Parallel to plane
    }

    float t = glm::dot(ring.position - ray.position, ring.normal) / denom;

    if (t < 0.0F) {
        return std::nullopt;
    }

    glm::vec3 hit_point = ray.position + (ray.direction * t);

    float dist_from_center = glm::length(hit_point - ring.position);

    if (glm::abs(dist_from_center - ring.radius) <= ring.thickness * 0.5F) {
        return t;
    }

    return std::nullopt;
}

struct AABB {
    glm::vec3 min;
    glm::vec3 max;

    [[nodiscard]] AABB transform(const glm::mat4& transform) const
    {
        return transform_fast(transform);
    }

    [[nodiscard]] bool intersection(const glm::vec3& position) const
    {
        return position.x >= min.x && position.y >= min.y && position.z >= min.z
            && position.x <= max.x && position.y <= max.y && position.z <= max.z;
    }

    [[nodiscard]] bool ray_intersection(const Ray& ray) const
    {
        glm::vec3 t0 = (min - ray.position) * ray.inv_direction;
        glm::vec3 t1 = (max - ray.position) * ray.inv_direction;

        glm::vec3 t_near = glm::min(t0, t1);
        glm::vec3 t_far = glm::max(t0, t1);

        float t_min = std::max({ t_near.x, t_near.y, t_near.z });
        float t_max = std::min({ t_far.x, t_far.y, t_far.z });

        return t_max >= std::max(0.0f, t_min);
    }

    [[nodiscard]] std::optional<glm::vec3> ray_intersection_point(const Ray& ray) const
    {
        glm::vec3 t0 = (min - ray.position) * ray.inv_direction;
        glm::vec3 t1 = (max - ray.position) * ray.inv_direction;

        glm::vec3 t_near = glm::min(t0, t1);
        glm::vec3 t_far = glm::max(t0, t1);

        float t_min = std::max({ t_near.x, t_near.y, t_near.z });
        float t_max = std::min({ t_far.x, t_far.y, t_far.z });

        if (t_max >= std::max(0.0f, t_min)) {
            float t_hit = std::max(0.0f, t_min);
            return ray.position + (t_hit * ray.direction);
        }

        return std::nullopt;
    }

private:
    // I know this function is correct
    [[nodiscard]] AABB transform_naive(const glm::mat4& transform)
    {
        AABB result;

        // Calculate all corners
        std::array<glm::vec3, 8> corners;
        corners[0] = transform * glm::vec4 { min, 1.0F };
        corners[1] = transform * glm::vec4 { max.x, min.y, min.z, 1.0F };
        corners[2] = transform * glm::vec4 { min.x, max.y, min.z, 1.0F };
        corners[3] = transform * glm::vec4 { min.x, min.y, max.z, 1.0F };
        corners[4] = transform * glm::vec4 { max.x, max.y, min.z, 1.0F };
        corners[5] = transform * glm::vec4 { max.x, max.y, max.z, 1.0F };
        corners[6] = transform * glm::vec4 { max.x, min.y, max.z, 1.0F };
        corners[7] = transform * glm::vec4 { min.x, max.y, max.z, 1.0F };

        // Find new min/max
        result.min = corners[0];
        result.max = corners[0];
        for (u32 i = 1; i < corners.size(); i++) {
            const auto& corner = corners.at(i);
            result.min = glm::min(corner, result.min);
            result.max = glm::max(corner, result.max);
        }

        return result;
    }

    [[nodiscard]] AABB transform_fast(const glm::mat4& transform) const
    {
        AABB result;

        // Start with the translation column of the matrix (Column 3)
        glm::vec3 translation = glm::vec3(transform[3]);
        result.min = translation;
        result.max = translation;

        // Loop through columns 0, 1, 2 (X, Y, Z basis vectors)
        for (int col = 0; col < 3; ++col) {
            for (int row = 0; row < 3; ++row) {
                float a = transform[col][row] * min[col];
                float b = transform[col][row] * max[col];

                if (a < b) {
                    result.min[row] += a;
                    result.max[row] += b;
                } else {
                    result.min[row] += b;
                    result.max[row] += a;
                }
            }
        }

        return result;
    }
};

}
