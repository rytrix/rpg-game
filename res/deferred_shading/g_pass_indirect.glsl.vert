#version 460 core
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoords;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;
out flat int DrawID;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    vec4 world_pos = model * vec4(inPos, 1.0);
    TexCoords = inTexCoords;

    Normal = mat3(transpose(inverse(model))) * inNormal;

    FragPos = world_pos.xyz;

    gl_Position = proj * view * world_pos;

    DrawID = gl_DrawID;
}