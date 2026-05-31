#pragma once

#include "../app_data.hpp"

#include "transform.hpp"

#include "event.hpp"

class Gizmo {
public:
    enum class State {
        Translation,
        Rotation,
        Scale,
    };
    Gizmo(GlobalAppData* app_data, Transform* transform);
    Gizmo(GlobalAppData* app_data);

    void on_event(Event event);
    void update();

    // This function only batches to the Line Renderer
    // app_data->debug_renderer.draw() has to be called after
    void draw();

    State m_state = State::Translation;
    f32 m_radius = 2.0;
    Transform* m_transform = nullptr;

private:
    GlobalAppData* m_app_data = nullptr;

    static constexpr f32 LINE_THICKNESS = 1.0F;

    struct PreviousHit {
        bool on_down;
        glm::vec3 hit;
        glm::vec3 normal;
        glm::vec3 direction;
        glm::vec3 outward_direction;

        glm::vec3 position;
        glm::vec3 scale;
        float prev_rotation;
    };

    PreviousHit m_prev_hit {};

    void batch_rotations(f32 radius);
    void batch_lines(f32 radius);

    void test_intersection();
    void test_intersection_lines();
    void test_intersection_rotation();
};