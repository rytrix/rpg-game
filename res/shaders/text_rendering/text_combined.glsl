#version 460 core
// Vertex Begin
out vec2 TexCoords;

struct GlyphInstance {
    vec4 rect;
    vec4 uvs;
};

layout(std430, binding = 0) readonly buffer VertexBuffer {
    GlyphInstance vertices[];
};

uniform mat4 projection;

const vec2 corners[6] = vec2[](
    vec2(0.0, 1.0), // top left
    vec2(0.0, 0.0), // bottom left
    vec2(1.0, 0.0), // bottom right
    vec2(0.0, 1.0), // top left
    vec2(1.0, 0.0), // bottom right
    vec2(1.0, 1.0)  // top right
);

void main() {
    uint glyph_index = uint(gl_VertexID) / 6;
    uint corner_index = uint(gl_VertexID) % 6;

    GlyphInstance glyph = vertices[glyph_index];
    vec2 local_pos = corners[corner_index];

    vec2 pos = glyph.rect.xy + (local_pos * glyph.rect.zw);
    gl_Position = projection * vec4(pos, 0.0, 1.0);
    TexCoords.x = mix(glyph.uvs.x, glyph.uvs.z, local_pos.x);
    TexCoords.y = mix(glyph.uvs.y, glyph.uvs.w, 1.0 - local_pos.y);
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
