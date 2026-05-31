#pragma once

#include "../utils/math/aabb.hpp"
#include "../utils/math/ray.hpp"

#include "buffer.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "vertex.hpp"

namespace Renderer {

class LineRenderer : public NoCopyNoMove {
public:
    LineRenderer() = default;
    LineRenderer(usize max_lines);
    ~LineRenderer();

    void init(usize max_lines = 10000);

    void add_line(glm::vec3 start, glm::vec3 end, glm::vec3 color);
    void add_line(const glm::mat4& transform, glm::vec3 begin, glm::vec3 end, glm::vec3 color);

    void add_aabb(const Utils::AABB& aabb, glm::vec3 color);
    void add_ray(const Utils::Ray& ray, float length, glm::vec3 color);

    static constexpr u32 DEFAULT_CIRCLE_SEGMENTS = 32;
    // Defaults to the Y axis with no rotation
    void add_circle(const glm::mat4& transform, f32 radius, glm::vec3 color);
    void add_circle(const glm::mat4& transform, f32 radius, u32 segments, glm::vec3 color);

    void draw(const Camera& camera);

private:
    bool initialized = false;

    usize m_max_vertices = 0;
    struct Vertex {
        // std430 expects vec4s
        glm::vec3 pos;
        u32 packed_color;
    };
    std::vector<Vertex> m_vertices;

    VertexArray m_vao;
    MappedBuffer m_ssbo;
    Shader m_shader;

    void setup_shader();
};

} // namespace Renderer
