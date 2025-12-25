#include "shader.hpp"

#include <fstream>

namespace Renderer {

namespace {

    constexpr std::size_t MAX_ERROR_LENGTH = 512;

    class Shader {
    public:
        Shader() = default;
        Shader(bool is_file, const char* shader, GLenum type);
        ~Shader();

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;
        Shader(Shader&&) = default;
        Shader& operator=(Shader&&) = default;

        void init(bool is_file, const char* shader, GLenum type);

        [[nodiscard]] GLuint get_id() const;

    private:
        GLuint m_id {};
        bool m_errors = true;

        [[nodiscard]] bool has_errors() const;
        static std::vector<char> from_file(const char* path);
    };

    Shader::Shader(bool is_file, const char* shader, GLenum type)
    {
        init(is_file, shader, type);
    }

    Shader::~Shader()
    {
        if (!m_errors) {
            glDeleteShader(m_id);
        }
    }

    void Shader::init(bool is_file, const char* shader, GLenum type)
    {
        m_id = glCreateShader(type);

        if (is_file) {
            std::vector<char> shader_text = from_file(shader);
            const char* data_text = shader_text.data();
            glShaderSource(m_id, 1, &data_text, nullptr);
        } else {
            glShaderSource(m_id, 1, &shader, nullptr);
        }

        glCompileShader(m_id);

        m_errors = has_errors();
    }

    [[nodiscard]] bool Shader::has_errors() const
    {
        int is_compiled = 0;
        glGetShaderiv(m_id, GL_COMPILE_STATUS, &is_compiled);

        if (is_compiled == GL_FALSE) {
            GLint max_length = 0;
            glGetShaderiv(m_id, GL_INFO_LOG_LENGTH, &max_length);

            std::vector<GLchar> error_log(max_length);
            glGetShaderInfoLog(m_id, max_length, &max_length, error_log.data());

            std::print("shader failed to compile: {}\n", error_log.data());
            return true;
        }

        return false;
    }

    std::vector<char> Shader::from_file(const char* path)
    {
        std::ifstream file(path, std::ios::in | std::ios::binary);
        if (!file) {
            std::println("Shader: Could not open file \"{}\"", path);
            return {};
        }

        std::vector<char> shader_text((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());

        shader_text.push_back('\0');

        return shader_text;
    }

    [[nodiscard]] GLuint Shader::get_id() const
    {
        return m_id;
    }

} // Anonymous namespace

ShaderProgram::ShaderProgram(ShaderInfo* shader_info, std::size_t shader_count)
{
    init(shader_info, shader_count);
}

ShaderProgram::~ShaderProgram()
{
    if (initialized) {
        if (!m_errors) {
            glDeleteProgram(m_id);
        }
        initialized = false;
    }
}

void ShaderProgram::init(ShaderInfo* shader_info, std::size_t shader_count)
{
    if (initialized) {
        throw std::runtime_error("ShaderProgram::init() attempting to reinit shader program");
    }

    static constexpr size_t MAX_SHADER_COUNT = 5;
    if (shader_count > MAX_SHADER_COUNT) {
        std::print("shader programs do not currently support more than 5 shaders\n");
        return;
    }
    std::array<Shader, MAX_SHADER_COUNT> shaders;

    m_id = glCreateProgram();

    for (std::size_t i = 0; i < shader_count; i++) {
        shaders.at(i).init(shader_info[i].is_file, shader_info[i].shader, shader_info[i].type);

        glAttachShader(m_id, shaders[i].get_id());
    }

    glLinkProgram(m_id);

    m_errors = errors_internal();

    if (m_errors) {
        throw std::runtime_error("Shader program has errors");
    }

    initialized = true;
}

void ShaderProgram::bind()
{
    glUseProgram(m_id);
}

void ShaderProgram::set_bool(const char* name, bool value)
{
    glUniform1i(glGetUniformLocation(m_id, name), value);
}

void ShaderProgram::set_int(const char* name, int value)
{
    glUniform1i(glGetUniformLocation(m_id, name), value);
}

void ShaderProgram::set_float(const char* name, float value)
{
    glUniform1f(glGetUniformLocation(m_id, name), value);
}

void ShaderProgram::set_vec2(const char* name, glm::vec2 value)
{
    glUniform2fv(glGetUniformLocation(m_id, name), 1, &value[0]);
}

void ShaderProgram::set_vec2s(const char* name, float value1, float value2)
{
    glUniform2f(glGetUniformLocation(m_id, name), value1, value2);
}

void ShaderProgram::set_vec3(const char* name, glm::vec3 value)
{
    glUniform3fv(glGetUniformLocation(m_id, name), 1, &value[0]);
}

void ShaderProgram::set_vec3s(const char* name, float value1, float value2, float value3)
{
    glUniform3f(glGetUniformLocation(m_id, name), value1, value2, value3);
}

void ShaderProgram::set_vec4(const char* name, glm::vec4 value)
{
    glUniform4fv(glGetUniformLocation(m_id, name), 1, &value[0]);
}

void ShaderProgram::set_vec4s(const char* name, float value1, float value2, float value3, float value4)
{
    glUniform4f(glGetUniformLocation(m_id, name), value1, value2, value3, value4);
}

void ShaderProgram::set_mat2(const char* name, glm::mat2 value)
{
    glUniformMatrix2fv(glGetUniformLocation(m_id, name), 1, GL_FALSE, &value[0][0]);
}

void ShaderProgram::set_mat3(const char* name, glm::mat3 value)
{
    glUniformMatrix3fv(glGetUniformLocation(m_id, name), 1, GL_FALSE, &value[0][0]);
}

void ShaderProgram::set_mat4(const char* name, glm::mat4 value)
{
    glUniformMatrix4fv(glGetUniformLocation(m_id, name), 1, GL_FALSE, &value[0][0]);
}

bool ShaderProgram::has_errors() const
{
    return m_errors;
}

bool ShaderProgram::errors_internal() const
{
    int program_linked = 0;

    glGetProgramiv(m_id, GL_LINK_STATUS, &program_linked);

    if (program_linked == GL_FALSE) {
        GLint max_length = 0;
        glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &max_length);

        std::vector<GLchar> error_log(max_length);
        glGetProgramInfoLog(m_id, max_length, &max_length, error_log.data());

        std::print("shader program failed to link: {}\n", error_log.data());
        return true;
    }

    return false;
}

} // namespace Renderer
