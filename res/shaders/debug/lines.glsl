#version 460

// Vertex Begin

layout(std430, binding = 0) readonly buffer VertexBuffer {
    vec3 vertices[];
};

uniform mat4 proj;
uniform mat4 view;

void main()
{
    vec3 in_pos = vertices[gl_VertexID];
    gl_Position = proj * view * vec4(in_pos, 1.0);
}

// Vertex End

#version 460
// Fragment Begin
out vec4 FragColor;

uniform vec3 color;

void main()
{
    FragColor = vec4(color, 1.0);
}

// Fragment End
