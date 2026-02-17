#version 460 core
// Vertex Begin
layout (location = 0) in vec3 inPos;

layout(binding = 1, std430) readonly buffer ssbo0 {
    mat4 models[];
};

uniform mat4 light_space_matrix;

void main()
{
    mat4 model = models[gl_InstanceID];
    gl_Position = light_space_matrix * model * vec4(inPos, 1.0);
}
// Vertex End

#version 460 core
// Fragment Begin
void main()
{
}
// Fragment End
