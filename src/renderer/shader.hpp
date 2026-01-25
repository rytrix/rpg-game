#pragma once

namespace Renderer {

struct ShaderInfo {
    bool is_file {};
    const char* shader {};
    GLenum type {};
};

class ShaderProgram : public NoCopyNoMove {
public:
    ShaderProgram() = default;
    ShaderProgram(ShaderInfo* shader_info, std::size_t shader_count);
    ~ShaderProgram();

    void init(ShaderInfo* shader_info, std::size_t shader_count);
    void bind();

    void set_bool(const char* name, bool value);
    void set_int(const char* name, int value);
    void set_float(const char* name, float value);
    void set_vec2(const char* name, glm::vec2 value);
    void set_vec2s(const char* name, float value1, float value2);
    void set_vec3(const char* name, glm::vec3 value);
    void set_vec3s(const char* name, float value1, float value2, float value3);
    void set_vec4(const char* name, glm::vec4 value);
    void set_vec4s(const char* name, float value1, float value2, float value3, float value4);
    void set_mat2(const char* name, glm::mat2 value);
    void set_mat3(const char* name, glm::mat3 value);
    void set_mat4(const char* name, glm::mat4 value);

    [[nodiscard]] bool has_errors() const;

private:
    bool initialized = false;

    GLuint m_id {};
    bool m_errors = true;

    [[nodiscard]] bool errors_internal() const;
};

} // namespace Renderer
