#pragma once

#include <glm/glm.hpp>

namespace Renderer {

#undef near
#undef far

class Camera {
public:
    enum class movement;
    Camera(float fov, float near, float far, uint32_t screenWidth, uint32_t screenHeight, glm::vec3 pos);
    void updateProj();
    void updateView();
    void updateAspect(float aspectRatio);
    void update();
    void move(movement direction, float deltaTime);
    void rotate(float xoffset, float yoffset);
    void updateVectors();
    void zoom(float yoffset);
    void setFirstMouse();

    glm::mat4 getProj() const;
    glm::mat4 getView() const;
    glm::mat4 getCombined() const;
    glm::mat4 getInverseView() const;
    glm::mat4 getInverseProj() const;
    glm::mat4 getInverseProjView() const;
    glm::vec3 getPos() const;
    glm::vec3 getFront() const;
    glm::vec3 getUp() const;
    glm::vec3 getRight() const;

    float getNear() const;
    float getFar() const;
    float getAspect() const;
    float getFov() const;

    enum class movement {
        forward,
        backward,
        left,
        right,
        up,
        down,
    };

    void setSpeed(float speed);

private:
    bool firstMouse = true;

    float fov;
    float near;
    float far;
    float aspectRatio;

    float speed;
    float sensitivity;

    float lastX;
    float lastY;
    float yaw;
    float pitch;

    glm::vec3 pos;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;

    glm::mat4 proj;
    glm::mat4 view;
};

} // namespace Renderer
