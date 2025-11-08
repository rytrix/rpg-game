#include "shader.hpp"

#include <cstdio>

namespace Renderer {

struct Shader {
    Shader(bool isFile, const char* shader, GLenum type);
    Shader();
    ~Shader();

    void init(bool isFile, const char* shader, GLenum type);

    NODISCARD GLuint getId() const { return id; }

private:
    GLuint id {};
    bool errors = true;

    bool hasErrors();
    char* fromFile(const char* path);
};

Shader::Shader(bool isFile, const char* shader, GLenum type)
{
    init(isFile, shader, type);
}

Shader::Shader() { }

Shader::~Shader()
{
    if (!errors)
        glDeleteShader(id);
}

void Shader::init(bool isFile, const char* shader, GLenum type)
{
    id = glCreateShader(type);

    if (isFile) {
        char* shaderText = fromFile(shader);
        glShaderSource(id, 1, &shaderText, NULL);
        free(shaderText);
    } else {
        glShaderSource(id, 1, &shader, NULL);
    }

    glCompileShader(id);

    errors = hasErrors();
}

bool Shader::hasErrors()
{
    int success;
    char infoLog[512];
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(id, 512, NULL, infoLog);
        fmt::print("shader failed to compile: {}\n", infoLog);
        return true;
    }

    return false;
}

char* Shader::fromFile(const char* path)
{
    FILE* file = fopen(path, "r");
    if (file == nullptr) {
        fmt::print("could not open file {}\n", path);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* shaderText = (char*)malloc(size + 1);
    if (shaderText == nullptr) {
        fmt::print("failed to malloc data for file {}\n", path);
    }

    fread(shaderText, size, 1, file);
    shaderText[size] = '\0';

    return shaderText;
}

ShaderProgram::ShaderProgram()
{
}

ShaderProgram::ShaderProgram(ShaderInfo* shaderInfo, int shaderCount)
{
    init(shaderInfo, shaderCount);
}

ShaderProgram::~ShaderProgram()
{
    if (!errors)
        glDeleteProgram(id);
}

void ShaderProgram::init(ShaderInfo* shaderInfo, int shaderCount)
{
    if (shaderCount > 5) {
        fmt::print("shader programs do not currently support more than 5 shaders\n");
        return;
    }
    Shader shaders[5];

    id = glCreateProgram();

    for (int i = 0; i < shaderCount; i++) {
        shaders[i].init((bool)shaderInfo[i].isFile, shaderInfo[i].shader, shaderInfo[i].type);
        glAttachShader(id, shaders[i].getId());
    }

    glLinkProgram(id);

    errors = hasErrorsInternal();
}

void ShaderProgram::bind()
{
    glUseProgram(id);
}

void ShaderProgram::setBool(const char* name, bool value)
{
    glUniform1i(glGetUniformLocation(id, name), value);
}

void ShaderProgram::setInt(const char* name, int value)
{
    glUniform1i(glGetUniformLocation(id, name), value);
}

void ShaderProgram::setFloat(const char* name, float value)
{
    glUniform1f(glGetUniformLocation(id, name), value);
}

void ShaderProgram::setVec2(const char* name, glm::vec2 value)
{
    glUniform2fv(glGetUniformLocation(id, name), 1, &value[0]);
}

void ShaderProgram::setVec2s(const char* name, float value1, float value2)
{
    glUniform2f(glGetUniformLocation(id, name), value1, value2);
}

void ShaderProgram::setVec3(const char* name, glm::vec3 value)
{
    glUniform3fv(glGetUniformLocation(id, name), 1, &value[0]);
}

void ShaderProgram::setVec3s(const char* name, float value1, float value2, float value3)
{
    glUniform3f(glGetUniformLocation(id, name), value1, value2, value3);
}

void ShaderProgram::setVec4(const char* name, glm::vec4 value)
{
    glUniform4fv(glGetUniformLocation(id, name), 1, &value[0]);
}

void ShaderProgram::setVec4s(const char* name, float value1, float value2, float value3, float value4)
{
    glUniform4f(glGetUniformLocation(id, name), value1, value2, value3, value4);
}

void ShaderProgram::setMat2(const char* name, glm::mat2 value)
{
    glUniformMatrix2fv(glGetUniformLocation(id, name), 1, GL_FALSE, &value[0][0]);
}

void ShaderProgram::setMat3(const char* name, glm::mat3 value)
{
    glUniformMatrix3fv(glGetUniformLocation(id, name), 1, GL_FALSE, &value[0][0]);
}

void ShaderProgram::setMat4(const char* name, glm::mat4 value)
{
    glUniformMatrix4fv(glGetUniformLocation(id, name), 1, GL_FALSE, &value[0][0]);
}

bool ShaderProgram::hasErrorsInternal()
{
    int success;
    char infoLog[512];
    glGetProgramiv(id, GL_LINK_STATUS, &success);

    if (!success) {
        glGetProgramInfoLog(id, 512, NULL, infoLog);
        fmt::print("shader program failed to link: {}\n", infoLog);
        return true;
    }

    return false;
}

} // namespace Renderer
