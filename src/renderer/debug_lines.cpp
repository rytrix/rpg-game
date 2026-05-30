#include "debug_lines.hpp"

#include "../renderer/shader_preprocessor.hpp"

#include "../utils/color.hpp"
#include "../utils/file.hpp"

namespace Renderer {

LineRenderer::LineRenderer(usize max_lines)
{
    init(max_lines);
}

void LineRenderer::init(usize max_lines)
{
    util_assert(initialized == false, "already initialized");

    m_max_vertices = max_lines * 2;

    m_vao.init();

    m_ssbo.init(3, m_max_vertices * sizeof(Vertex));

    m_vertices.reserve(m_max_vertices);

    setup_shader();

    initialized = true;
}

LineRenderer::~LineRenderer()
{
    if (initialized) {
        initialized = false;
    }
}

void LineRenderer::add_line(glm::vec3 begin, glm::vec3 end, glm::vec3 color)
{
    util_assert(initialized == true, "not initialized");
    if (m_vertices.size() + 2 > m_max_vertices) {
        LOG_ERROR("Exceeding max number of lines, new lines are not being added");
        return;
    }
    m_vertices.push_back({ begin, Color::pack(color) });
    m_vertices.push_back({ end, Color::pack(color) });
}

void LineRenderer::add_line(const glm::mat4& transform, glm::vec3 begin, glm::vec3 end, glm::vec3 color)
{
    util_assert(initialized == true, "not initialized");
    if (m_vertices.size() + 2 > m_max_vertices) {
        LOG_ERROR("Exceeding max number of lines, new lines are not being added");
        return;
    }
    m_vertices.push_back({ transform * glm::vec4(begin, 1.0), Color::pack(color) });
    m_vertices.push_back({ transform * glm::vec4(end, 1.0), Color::pack(color) });
}

void LineRenderer::add_aabb(const Utils::AABB& aabb, glm::vec3 color)
{
    util_assert(initialized == true, "not initialized");

    glm::vec3 c[8] = {
        { aabb.min.x, aabb.min.y, aabb.min.z },
        { aabb.max.x, aabb.min.y, aabb.min.z },
        { aabb.max.x, aabb.max.y, aabb.min.z },
        { aabb.min.x, aabb.max.y, aabb.min.z },
        { aabb.min.x, aabb.min.y, aabb.max.z },
        { aabb.max.x, aabb.min.y, aabb.max.z },
        { aabb.max.x, aabb.max.y, aabb.max.z },
        { aabb.min.x, aabb.max.y, aabb.max.z }
    };

    // Bottom face
    add_line(c[0], c[1], color);
    add_line(c[1], c[2], color);
    add_line(c[2], c[3], color);
    add_line(c[3], c[0], color);

    // Top face
    add_line(c[4], c[5], color);
    add_line(c[5], c[6], color);
    add_line(c[6], c[7], color);
    add_line(c[7], c[4], color);

    // Vertical pillars
    add_line(c[0], c[4], color);
    add_line(c[1], c[5], color);
    add_line(c[2], c[6], color);
    add_line(c[3], c[7], color);
}

void LineRenderer::add_ray(const Utils::Ray& ray, float length, glm::vec3 color)
{
    util_assert(initialized == true, "not initialized");
    glm::vec3 end = ray.position + (ray.direction * length);
    add_line(ray.position, end, color);
}

void LineRenderer::add_circle(const glm::mat4& transform, f32 radius, glm::vec3 color)
{
    util_assert(initialized == true, "not initialized");
    add_circle(transform, radius, DEFAULT_CIRCLE_SEGMENTS, color);
}

void LineRenderer::add_circle(const glm::mat4& transform, f32 radius, u32 segments, glm::vec3 color)
{
    util_assert(initialized == true, "not initialized");

    float angle_step = glm::two_pi<float>() / static_cast<float>(segments);

    glm::vec4 first_local(radius, 0.0f, 0.0f, 1.0f);
    glm::vec3 first_world = transform * first_local;

    glm::vec3 prev_world = first_world;

    for (u32 i = 1; i <= segments + 1; ++i) {
        float angle = static_cast<float>(i) * angle_step;

        glm::vec4 local_pos(
            glm::cos(angle) * radius,
            glm::sin(angle) * radius,
            0.0f,
            1.0f);

        glm::vec3 current_world = transform * local_pos;

        add_line(prev_world, current_world, color);
        prev_world = current_world;
    }
}

void LineRenderer::draw(const Camera& camera)
{
    if (m_vertices.size() == 0) {
        return;
    }
    util_assert(initialized == true, "not initialized");
    m_vao.bind();
    m_shader.bind();

    // m_shader.set_vec3("color", color);
    m_shader.set_mat4("proj", camera.get_proj());
    m_shader.set_mat4("view", camera.get_view());

    void* ssbo_ptr = m_ssbo.get_ptr();
    memcpy(ssbo_ptr, &m_vertices[0], m_vertices.size() * sizeof(Vertex));

    glDisable(GL_DEPTH_TEST);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssbo.get_id());
    glDrawArrays(GL_LINES, 0, m_vertices.size());

    glEnable(GL_DEPTH_TEST);

    m_vertices.clear();
    m_ssbo.increment_frame();
}

void LineRenderer::setup_shader()
{
    ShaderInfoData<2> out;

    std::vector<char> text_shader_file = read_file<char>("res/shaders/debug/lines.glsl");
    std::string_view text_shader_file_view = { text_shader_file.data(), text_shader_file.size() };

    // Vertex Shader
    out.data.at(0) = "#version 460 core\n";
    out.data.at(0) += get_lines_between_delims(text_shader_file_view, "// Vertex Begin", "// Vertex End");

    // Fragment Shader
    out.data.at(1) += "#version 460 core\n";
    out.data.at(1) += get_lines_between_delims(text_shader_file_view, "// Fragment Begin", "// Fragment End");

    out.populate_info();

    m_shader.init(out.info.data(), out.info.size());
}

} // namespace Renderer
