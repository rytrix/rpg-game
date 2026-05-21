#pragma once

namespace Renderer {

struct Ray {
    glm::vec3 position;
    glm::vec3 direction;
};

struct AABB {
    glm::vec3 min;
    glm::vec3 max;

    [[nodiscard]] bool intersection(glm::vec3 position) const
    {
        return position.x >= min.x && position.y >= min.y && position.z >= min.z
            && position.x <= max.x && position.y <= max.y && position.z <= max.z;
    }

    [[nodiscard]] bool ray_intersection(const Ray& ray) const
    {
        glm::vec3 inv_dir = 1.0f / ray.direction;

        glm::vec3 t0 = (min - ray.position) * inv_dir;
        glm::vec3 t1 = (max - ray.position) * inv_dir;

        glm::vec3 t_near = glm::min(t0, t1);
        glm::vec3 t_far = glm::max(t0, t1);

        float t_min = std::max({ t_near.x, t_near.y, t_near.z });
        float t_max = std::min({ t_far.x, t_far.y, t_far.z });

        return t_max >= std::max(0.0f, t_min);
    }
};

}
