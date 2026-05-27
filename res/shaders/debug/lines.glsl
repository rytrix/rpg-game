#version 460

// Vertex Begin

struct Vertex {
    vec3 pos;
    uint color;
};

out vec4 color;

layout(std430, binding = 0) readonly buffer VertexBuffer {
    Vertex vertices[];
};

uniform mat4 proj;
uniform mat4 view;

void main()
{
    Vertex in_vert = vertices[gl_VertexID];
    gl_Position = proj * view * vec4(in_vert.pos, 1.0);
    
    color.r = float((in_vert.color)       & 0xFFu) / 255.0;
    color.g = float((in_vert.color >> 8)  & 0xFFu) / 255.0;
    color.b = float((in_vert.color >> 16) & 0xFFu) / 255.0;
    color.a = float((in_vert.color >> 24) & 0xFFu) / 255.0;
}

// Vertex End

#version 460
// Fragment Begin
in vec4 color;
out vec4 FragColor;

// uniform vec3 color;

void main()
{
    FragColor = color;
}

// Fragment End
