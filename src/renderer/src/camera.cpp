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
    : fov(glm::radians(fov))
    , near(near)
    , far(far)
    , aspectRatio((float)screenWidth / (float)screenHeight)
    , speed(defaults::speed)
    , sensitivity(defaults::sensitivity)
    , yaw(defaults::yaw)
    , pitch(defaults::pitch)
    , pos(pos)
{
    front = glm::vec3(0.0f, 0.0f, -1.0f);
    up = defaults::worldUp;
    right = glm::normalize(glm::cross(front, up));

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
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    this->front = glm::normalize(front);

    right = glm::normalize(glm::cross(this->front, defaults::worldUp));
    up = glm::normalize(glm::cross(right, this->front));
}

void Camera::updateProj()
{
    proj = glm::perspective(fov, aspectRatio, near, far);
}

void Camera::updateView()
{
    view = glm::lookAt(pos, pos + front, up);
}

void Camera::updateAspect(float aspectRatio)
{
    this->aspectRatio = aspectRatio;
    update();
}

void Camera::move(movement direction, float deltaTime)
{
    float velocity = speed * deltaTime;
    switch (direction) {
    case movement::forward:
        pos += front * velocity;
        break;
    case movement::backward:
        pos -= front * velocity;
        break;
    case movement::left:
        pos -= right * velocity;
        break;
    case movement::right:
        pos += right * velocity;
        break;
    case movement::up:
        pos += up * velocity;
        break;
    case movement::down:
        pos -= up * velocity;
        break;
    }
}

void Camera::rotate(float xPos, float yPos)
{
    if (firstMouse) {
        firstMouse = false;
        lastX = xPos;
        lastY = yPos;
        return;
    }
    float xoffset = xPos - lastX;
    float yoffset = lastY - yPos;
    lastX = xPos;
    lastY = yPos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    update();
}

void Camera::zoom(float yoffset)
{
    if (fov >= 1.0f && fov <= defaults::maxFov)
        fov -= yoffset;
    if (fov <= 1.0f)
        fov = 1.0f;
    if (fov >= defaults::maxFov)
        fov = defaults::maxFov;

    updateProj();
}

glm::mat4 Camera::getProj() const
{
    return proj;
}

glm::mat4 Camera::getView() const
{
    return view;
}

glm::mat4 Camera::getCombined() const
{
    return proj * view;
}

glm::mat4 Camera::getInverseView() const
{
    return glm::inverse(view);
}

glm::mat4 Camera::getInverseProj() const
{
    return glm::inverse(proj);
}

glm::mat4 Camera::getInverseProjView() const
{
    return glm::inverse(proj * view);
}

glm::vec3 Camera::getPos() const
{
    return pos;
}

glm::vec3 Camera::getFront() const
{
    return front;
}

glm::vec3 Camera::getUp() const
{
    return up;
}

glm::vec3 Camera::getRight() const
{
    return right;
}
float Camera::getNear() const
{
    return near;
}

float Camera::getFar() const
{
    return far;
}

float Camera::getAspect() const
{
    return aspectRatio;
}

float Camera::getFov() const
{
    return fov;
}

void Camera::setFirstMouse()
{
    firstMouse = true;
}

void Camera::setSpeed(float speed)
{
    this->speed = speed;
}

} // namespace Renderer
