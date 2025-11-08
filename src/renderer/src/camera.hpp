#pragma once

namespace Renderer {

#undef near
#undef far

class Camera {
public:
    enum class Movement;
    Camera(float fov, float near, float far, float aspect_ratio, glm::vec3 pos);
    void update_proj();
    void update_view();
    void update_aspect(float aspect_ratio);
    void update();
    void move(Movement direction, float delta_time);
    void rotate(float x_offset, float y_offset);
    void update_vectors();
    void zoom(float y_offset);
    void set_first_mouse();

    [[nodiscard]] glm::mat4 get_proj() const;
    [[nodiscard]] glm::mat4 get_view() const;
    [[nodiscard]] glm::mat4 get_combined() const;
    [[nodiscard]] glm::mat4 get_inverse_view() const;
    [[nodiscard]] glm::mat4 get_inverse_proj() const;
    [[nodiscard]] glm::mat4 get_inverse_proj_view() const;
    [[nodiscard]] glm::vec3 get_pos() const;
    [[nodiscard]] glm::vec3 get_front() const;
    [[nodiscard]] glm::vec3 get_up() const;
    [[nodiscard]] glm::vec3 get_right() const;

    [[nodiscard]] float get_near() const;
    [[nodiscard]] float get_far() const;
    [[nodiscard]] float get_aspect() const;
    [[nodiscard]] float get_fov() const;

    enum class Movement {
        Forward,
        Backward,
        Left,
        Right,
        Up,
        Down,
    };

    static constexpr float calculate_aspect_ratio(float width, float height)
    {
        return width / height;
    }

    void set_speed(float speed);

private:
    bool m_first_mouse = true;

    float m_fov {};
    float m_near {};
    float m_far {};
    float m_aspect_ratio {};

    float m_speed {};
    float m_sensitivity {};

    float m_last_x {};
    float m_last_y {};
    float m_yaw {};
    float m_pitch {};

    glm::vec3 m_pos {};
    glm::vec3 m_front {};
    glm::vec3 m_up {};
    glm::vec3 m_right {};

    glm::mat4 m_proj {};
    glm::mat4 m_view {};
};

} // namespace Renderer
