#version 460 core
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoords;
layout (location = 3) in vec3 inTangent;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;
out vec3 Tangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    TexCoords = inTexCoords;

    mat3 transposed_model = mat3(transpose(inverse(model)));
    Normal = transposed_model * inNormal;
    Tangent = transposed_model * inTangent;

    FragPos = vec3(model * vec4(inPos, 1.0));
    gl_Position = proj * view * model * vec4(inPos, 1.0);
}
