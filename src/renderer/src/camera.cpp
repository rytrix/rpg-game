#include "camera.hpp"

namespace Renderer {

namespace defaults {
    static constexpr float yaw = -90.0f;
    static constexpr float pitch = 0.0f;
    static constexpr float speed = 1.0f;
    static constexpr float maxFov = 110.0f;
    static constexpr float sensitivity = 0.1f;
    static constexpr glm::vec3 worldUp = { 0.0f, 1.0f, 0.0f };
}

Camera::Camera(float fov, float near, float far, uint32_t screenWidth, uint32_t screenHeight, glm::vec3 pos)
    : m_fov(glm::radians(fov))
    , m_near(near)
    , m_far(far)
    , m_aspect_ratio((float)screenWidth / (float)screenHeight)
    , m_speed(defaults::speed)
    , m_sensitivity(defaults::sensitivity)
    , m_yaw(defaults::yaw)
    , m_pitch(defaults::pitch)
    , m_pos(pos)
{
    m_front = glm::vec3(0.0f, 0.0f, -1.0f);
    m_up = defaults::worldUp;
    m_right = glm::normalize(glm::cross(m_front, m_up));

    update();
}

void Camera::update()
{
    updateVectors();
    updateProj();
    updateView();
}

void Camera::updateVectors()
{
    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(front);

    m_right = glm::normalize(glm::cross(this->m_front, defaults::worldUp));
    m_up = glm::normalize(glm::cross(m_right, this->m_front));
}

void Camera::updateProj()
{
    m_proj = glm::perspective(m_fov, m_aspect_ratio, m_near, m_far);
}

void Camera::updateView()
{
    m_view = glm::lookAt(m_pos, m_pos + m_front, m_up);
}

void Camera::updateAspect(float aspectRatio)
{
    this->m_aspect_ratio = aspectRatio;
    update();
}

void Camera::move(movement direction, float deltaTime)
{
    float velocity = m_speed * deltaTime;
    switch (direction) {
        case movement::forward:
            m_pos += m_front * velocity;
            break;
        case movement::backward:
            m_pos -= m_front * velocity;
            break;
        case movement::left:
            m_pos -= m_right * velocity;
            break;
        case movement::right:
            m_pos += m_right * velocity;
            break;
        case movement::up:
            m_pos += m_up * velocity;
            break;
        case movement::down:
            m_pos -= m_up * velocity;
            break;
    }
}

void Camera::rotate(float xPos, float yPos)
{
    if (m_first_mouse) {
        m_first_mouse = false;
        m_last_x = xPos;
        m_last_y = yPos;
        return;
    }
    float xoffset = xPos - m_last_x;
    float yoffset = m_last_y - yPos;
    m_last_x = xPos;
    m_last_y = yPos;

    xoffset *= m_sensitivity;
    yoffset *= m_sensitivity;

    m_yaw += xoffset;
    m_pitch += yoffset;

    if (m_pitch > 89.0f)
        m_pitch = 89.0f;
    if (m_pitch < -89.0f)
        m_pitch = -89.0f;

    update();
}

void Camera::zoom(float yoffset)
{
    if (m_fov >= 1.0f && m_fov <= defaults::maxFov)
        m_fov -= yoffset;
    if (m_fov <= 1.0f)
        m_fov = 1.0f;
    if (m_fov >= defaults::maxFov)
        m_fov = defaults::maxFov;

    updateProj();
}

glm::mat4 Camera::getProj() const
{
    return m_proj;
}

glm::mat4 Camera::getView() const
{
    return m_view;
}

glm::mat4 Camera::getCombined() const
{
    return m_proj * m_view;
}

glm::mat4 Camera::getInverseView() const
{
    return glm::inverse(m_view);
}

glm::mat4 Camera::getInverseProj() const
{
    return glm::inverse(m_proj);
}

glm::mat4 Camera::getInverseProjView() const
{
    return glm::inverse(m_proj * m_view);
}

glm::vec3 Camera::getPos() const
{
    return m_pos;
}

glm::vec3 Camera::getFront() const
{
    return m_front;
}

glm::vec3 Camera::getUp() const
{
    return m_up;
}

glm::vec3 Camera::getRight() const
{
    return m_right;
}
float Camera::getNear() const
{
    return m_near;
}

float Camera::getFar() const
{
    return m_far;
}

float Camera::getAspect() const
{
    return m_aspect_ratio;
}

float Camera::getFov() const
{
    return m_fov;
}

void Camera::setFirstMouse()
{
    m_first_mouse = true;
}

void Camera::setSpeed(float speed)
{
    this->m_speed = speed;
}

} // namespace Renderer
