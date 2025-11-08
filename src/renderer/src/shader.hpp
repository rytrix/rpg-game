#pragma once

namespace Renderer {

enum ShaderInfoEnum {
	Text = 0,
	File = 1,
};

struct ShaderInfo {
    const ShaderInfoEnum isFile{};
    const char* shader{};
    GLenum type{};
};

struct ShaderProgram {
    ShaderProgram();
	ShaderProgram(ShaderInfo* shaderInfo, int shaderCount);
	~ShaderProgram();

    void init(ShaderInfo* shaderInfo, int shaderCount);
	void bind();

	void setBool(const char* name, bool value);
	void setInt(const char* name, int value);
	void setFloat(const char* name, float value);
	void setVec2(const char* name, glm::vec2 value);
	void setVec2s(const char* name, float value1, float value2);
	void setVec3(const char* name, glm::vec3 value);
	void setVec3s(const char* name, float value1, float value2, float value3);
	void setVec4(const char* name, glm::vec4 value);
	void setVec4s(const char* name, float value1, float value2, float value3, float value4);
	void setMat2(const char* name, glm::mat2 value);
	void setMat3(const char* name, glm::mat3 value);
	void setMat4(const char* name, glm::mat4 value);

    bool hasErrors() { return errors; }

private:
	GLuint id{};
	bool errors = true;

	bool hasErrorsInternal();
};

} // namespace Renderer
