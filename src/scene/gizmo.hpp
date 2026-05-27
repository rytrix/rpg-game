#pragma once

#include "../app_data.hpp"

#include "transform.hpp"

class Gizmo {
public:
    enum class State {
        Translation,
        Rotation,
        Scale,
    };
    Gizmo(GlobalAppData* app_data);

    void on_event();
    void update();

    void test_intersection_rotation(Transform* transform);
    void test_reset();

    // This function only batches to the Line Renderer
    // app_data->debug_renderer.draw() has to be called after
    void draw();

    void set_position(glm::vec3 position);

    State m_state { State::Translation };
    f32 m_radius = 2.0;

private:
    GlobalAppData* m_data = nullptr;

    glm::vec3 m_position {};

    struct PreviousHit {
        bool reset;
        glm::vec3 hit;
        glm::vec3 normal;
        float prev_rotation;
    };

    PreviousHit m_prev_hit;

    void batch_rotations(f32 radius);
    void batch_lines(f32 radius);
};