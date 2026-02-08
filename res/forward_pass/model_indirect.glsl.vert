#version 460 core
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoords;
layout (location = 3) in vec3 inTangent;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;
out vec3 Tangent;
out flat int DrawID;

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

    mat3 transposed_model = mat3(transpose(inverse(model)));
    Normal = transposed_model * inNormal;
    Tangent = transposed_model * inTangent;

    FragPos = world_pos.xyz;

    DrawID = gl_DrawID;

    gl_Position = proj * view * world_pos;
}