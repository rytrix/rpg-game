#pragma once

#include "aabb.hpp"
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

    void add_line(glm::vec3 start, glm::vec3 end);
    void add_aabb(const AABB& aabb);
    void add_ray(const Ray& ray, float length);
    void add_circle(const glm::mat4& transform, f32 radius, u32 segments = 32);

    void draw(glm::vec3 color, const Camera& camera);

private:
    bool initialized = false;

    usize m_max_vertices = 0;
    struct Vertex {
        // std430 expects vec4s
        alignas(16) glm::vec3 pos;
    };
    std::vector<Vertex> m_vertices;

    VertexArray m_vao;
    MappedBuffer m_ssbo;
    Shader m_shader;

    void setup_shader();
};

} // namespace Renderer
