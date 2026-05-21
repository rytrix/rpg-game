#version 460 core
// Vertex Begin
out vec2 TexCoords;

layout(std430, binding = 0) readonly buffer VertexBuffer {
    vec4 vertices[];
};

uniform mat4 projection;

void main() {
    vec4 vertex = vertices[gl_VertexID];
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
// Vertex End

#version 460 core
// Fragment Begin
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text_atlas;
uniform vec3 text_color;

void main() {
    vec4 sampled = vec4(vec3(1.0), texture(text_atlas, TexCoords).r);
    color = vec4(text_color, 1.0) * sampled;
}
// Fragment End
