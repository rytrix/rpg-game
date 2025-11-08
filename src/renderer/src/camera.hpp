#pragma once

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
    bool m_first_mouse = true;

    float m_fov;
    float m_near;
    float m_far;
    float m_aspect_ratio;

    float m_speed;
    float m_sensitivity;

    float m_last_x;
    float m_last_y;
    float m_yaw;
    float m_pitch;

    glm::vec3 m_pos;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;

    glm::mat4 m_proj;
    glm::mat4 m_view;
};

} // namespace Renderer
