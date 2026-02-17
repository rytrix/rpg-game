#version 460 core
// Vertex Begin
layout (location = 0) in vec3 inPos;

layout(binding = 1, std430) readonly buffer ssbo0 {
    mat4 models[];
};

uniform mat4 light_space_matrix;

out vec4 FragPos;

void main()
{
    mat4 model = models[gl_InstanceID];
    FragPos = model * vec4(inPos, 1.0);
    gl_Position = light_space_matrix * FragPos;
}
// Vertex End

#version 460 core
// Fragment Begin
in vec4 FragPos;

uniform vec3 light_pos;
uniform float far_plane;

void main()
{
    float light_distance = length(FragPos.xyz - light_pos);

    // map to [0;1] range
    light_distance = light_distance / far_plane;

    gl_FragDepth = light_distance;
}
// Fragment End
