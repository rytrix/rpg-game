#include "camera.hpp"

namespace Renderer {

namespace defaults {
    static constexpr float yaw = -90.0F;
    static constexpr float pitch = 0.0F;
    static constexpr float speed = 1.0F;
    static constexpr float max_fov = 110.0F;
    static constexpr float sensitivity = 0.1F;
    static constexpr glm::vec3 world_up = { 0.0F, 1.0F, 0.0F };
    static constexpr glm::vec3 world_front = { 0.0F, 0.0F, -1.0F };
}

Camera::Camera(float fov, float near, float far, float aspect_ratio, glm::vec3 pos)
    : m_fov(glm::radians(fov))
    , m_near(near)
    , m_far(far)
    , m_aspect_ratio(aspect_ratio)
    , m_speed(defaults::speed)
    , m_sensitivity(defaults::sensitivity)
    , m_yaw(defaults::yaw)
    , m_pitch(defaults::pitch)
    , m_pos(pos)
    , m_front(defaults::world_front)
    , m_up(defaults::world_up)
{
    m_right = glm::normalize(glm::cross(m_front, m_up));

    update();
}

void Camera::update()
{
    update_vectors();
    update_proj();
    update_view();
}

void Camera::update_vectors()
{
    glm::vec3 front;
    front.x = glm::cos(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch));
    front.y = glm::sin(glm::radians(m_pitch));
    front.z = glm::sin(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch));
    m_front = glm::normalize(front);

    m_right = glm::normalize(glm::cross(this->m_front, defaults::world_up));
    m_up = glm::normalize(glm::cross(m_right, this->m_front));
}

void Camera::update_proj()
{
    m_proj = glm::perspective(m_fov, m_aspect_ratio, m_near, m_far);
}

void Camera::update_view()
{
    m_view = glm::lookAt(m_pos, m_pos + m_front, m_up);
}

void Camera::update_aspect(float aspect_ratio)
{
    this->m_aspect_ratio = aspect_ratio;
    update_proj();
}

void Camera::move(Movement direction, float delta_time)
{
    float velocity = m_speed * delta_time;
    switch (direction) {
        case Movement::Forward:
            m_pos += m_front * velocity;
            break;
        case Movement::Backward:
            m_pos -= m_front * velocity;
            break;
        case Movement::Left:
            m_pos -= m_right * velocity;
            break;
        case Movement::Right:
            m_pos += m_right * velocity;
            break;
        case Movement::Up:
            m_pos += m_up * velocity;
            break;
        case Movement::Down:
            m_pos -= m_up * velocity;
            break;
    }
}

void Camera::rotate(float x_pos, float y_pos)
{
    if (m_first_mouse) {
        m_first_mouse = false;
        m_last_x = x_pos;
        m_last_y = y_pos;
        return;
    }
    float xoffset = x_pos - m_last_x;
    float yoffset = m_last_y - y_pos;
    m_last_x = x_pos;
    m_last_y = y_pos;

    xoffset *= m_sensitivity;
    yoffset *= m_sensitivity;

    m_yaw += xoffset;
    m_pitch += yoffset;

    m_pitch = std::clamp(m_pitch, -89.0F, 89.0F);
    // if (m_pitch > 89.0f)
    //     m_pitch = 89.0f;
    // if (m_pitch < -89.0f)
    //     m_pitch = -89.0f;

    update();
}

void Camera::zoom(float y_offset)
{
    if (m_fov >= 1.0F && m_fov <= defaults::max_fov) {
        m_fov -= y_offset;
    }
    m_fov = std::max(m_fov, 1.0F);

    m_fov = std::min(m_fov, defaults::max_fov);

    update_proj();
}

glm::mat4 Camera::get_proj() const
{
    return m_proj;
}

glm::mat4 Camera::get_view() const
{
    return m_view;
}

glm::mat4 Camera::get_combined() const
{
    return m_proj * m_view;
}

glm::mat4 Camera::get_inverse_view() const
{
    return glm::inverse(m_view);
}

glm::mat4 Camera::get_inverse_proj() const
{
    return glm::inverse(m_proj);
}

glm::mat4 Camera::get_inverse_proj_view() const
{
    return glm::inverse(m_proj * m_view);
}

glm::vec3 Camera::get_pos() const
{
    return m_pos;
}

glm::vec3 Camera::get_front() const
{
    return m_front;
}

glm::vec3 Camera::get_up() const
{
    return m_up;
}

glm::vec3 Camera::get_right() const
{
    return m_right;
}
float Camera::get_near() const
{
    return m_near;
}

float Camera::get_far() const
{
    return m_far;
}

float Camera::get_aspect() const
{
    return m_aspect_ratio;
}

float Camera::get_fov() const
{
    return m_fov;
}

void Camera::set_first_mouse()
{
    m_first_mouse = true;
}

void Camera::set_speed(float speed)
{
    this->m_speed = speed;
}

} // namespace Renderer
