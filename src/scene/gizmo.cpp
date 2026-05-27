#include "gizmo.hpp"

#include "../utils/color.hpp"

#include "transform.hpp"

Gizmo::Gizmo(GlobalAppData* app_data)
    : m_data(app_data)
{
}

void Gizmo::test_intersection_rotation(Transform* transform)
{
    auto ray = Utils::ray_from_mouse(m_data);

    if (m_prev_hit.reset) {
        Utils::Ring ring {};
        ring.position = m_position;
        ring.normal = glm::vec3(1.0, 0.0, 0.0);
        ring.radius = m_radius;
        ring.thickness = 0.3;

        auto result = Utils::intersect_ray_ring(ray, ring);
        if (result.has_value()) {
            glm::vec3 hit = result.value();
            std::println("Gizmo intersection worked {}", glm::length(ray.position - hit));
            m_prev_hit.hit = hit;
            m_prev_hit.reset = false;
            m_prev_hit.normal = ring.normal;
            m_prev_hit.prev_rotation = 0.0F;
            return;
        }

        ring.normal = glm::vec3(0.0, 1.0, 0.0);
        result = Utils::intersect_ray_ring(ray, ring);
        if (result.has_value()) {
            glm::vec3 hit = result.value();
            std::println("Gizmo intersection worked {}", glm::length(ray.position - hit));
            m_prev_hit.hit = hit;
            m_prev_hit.reset = false;
            m_prev_hit.normal = ring.normal;
            m_prev_hit.prev_rotation = 0.0F;
            return;
        }

        ring.normal = glm::vec3(0.0, 0.0, 1.0);
        result = Utils::intersect_ray_ring(ray, ring);
        if (result.has_value()) {
            glm::vec3 hit = result.value();
            std::println("Gizmo intersection worked {}", glm::length(ray.position - hit));
            m_prev_hit.hit = hit;
            m_prev_hit.reset = false;
            m_prev_hit.normal = ring.normal;
            m_prev_hit.prev_rotation = 0.0F;
            return;
        }
    } else {
        Utils::Plane plane {};
        plane.normal = m_prev_hit.normal;
        plane.position = m_position;

        auto result = Utils::intersect_ray_plane(ray, plane);
        if (result.has_value()) {
            auto hit = result.value();
            // Use trig to find the angle between that and the new hitpoint
            // Add the angle to the stored translation

            glm::vec3 a = glm::normalize(m_prev_hit.hit - m_position);
            glm::vec3 b = glm::normalize(hit - m_position);

            float dot = glm::dot(a, b);
            dot = glm::clamp(dot, -1.0F, 1.0F);

            float angle = glm::degrees(glm::acos(dot));

            // If the cross product is opposite the rings normal flip the sign
            glm::vec3 cross_prod = glm::cross(a, b);
            if (glm::dot(cross_prod, plane.normal) < 0.0F) {
                angle = -angle;
            }

            std::println("Rotate {} degrees, axis: {} {} {}", angle, m_prev_hit.normal.x, m_prev_hit.normal.y, m_prev_hit.normal.z);
            auto angle_difference = angle - m_prev_hit.prev_rotation;
            m_prev_hit.prev_rotation = angle;
            transform->rotate(angle_difference, plane.normal);
        }
    }
}

void Gizmo::test_reset()
{
    // Event for keyup to reset the internal state
    m_prev_hit.reset = true;
}

void Gizmo::draw()
{
    switch (m_state) {
        case State::Translation:
        case State::Scale:
            batch_lines(m_radius);
            break;
        case State::Rotation:
            batch_rotations(m_radius);
            break;
    }
}

void Gizmo::set_position(glm::vec3 position)
{
    m_position = position;
}

void Gizmo::batch_rotations(f32 radius)
{
    Transform transform;
    transform.set_position(m_position);
    m_data->debug_renderer.add_circle(transform.get_model(), radius, Color::Blue);
    transform.set_euler_angles(glm::vec3(90.0, 0.0, 0.0));
    m_data->debug_renderer.add_circle(transform.get_model(), radius, Color::Green);
    transform.set_euler_angles(glm::vec3(0.0, 90.0, 0.0));
    m_data->debug_renderer.add_circle(transform.get_model(), radius, Color::Red);
}

void Gizmo::batch_lines(f32 radius)
{
    Transform transform;
    transform.set_position(m_position);
    m_data->debug_renderer.add_line(transform.get_model(),
        glm::vec3(0.0, -radius, 0.0), glm::vec3(0.0, radius, 0.0), Color::Blue);
    m_data->debug_renderer.add_line(transform.get_model(),
        glm::vec3(-radius, 0.0, 0.0), glm::vec3(radius, 0.0, 0.0), Color::Green);
    m_data->debug_renderer.add_line(transform.get_model(),
        glm::vec3(0.0, 0.0, -radius), glm::vec3(0.0, 0.0, radius), Color::Red);
}