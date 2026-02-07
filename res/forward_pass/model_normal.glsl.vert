#version 460 core
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoords;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;

// uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

layout(binding = 1, std430) readonly buffer ssbo0 {
    mat4 models[];
};

void main()
{
    mat4 model = models[gl_InstanceID];

    vec4 world_pos = model * vec4(inPos, 1.0);
    TexCoords = inTexCoords;

    Normal = mat3(transpose(inverse(model))) * inNormal;

    FragPos = world_pos.xyz;

    gl_Position = proj * view * world_pos;
}