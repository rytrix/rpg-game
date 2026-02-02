#version 460 core
#extension GL_ARB_bindless_texture : require
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;
in flat int DrawID;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
};

layout(binding = 1, std430) readonly buffer ssbo1 {
    sampler2D diffuse[];
};

layout(binding = 2, std430) readonly buffer ssbo2 {
    sampler2D specular[];
};

// uniform Material material;

uniform int diffuse_max_textures;
uniform int specular_max_textures;

void main()
{
    gPosition = FragPos;
    gNormal = normalize(Normal);
    if (DrawID < diffuse_max_textures) {
        gAlbedo.rgb = texture(diffuse[DrawID], TexCoords).rgb;
    } else {
        gAlbedo.rgb = vec3(0.0);
    }

    if (DrawID < diffuse_max_textures) {
        gAlbedo.a = texture(specular[DrawID], TexCoords).r;
    } else {
        gAlbedo.a = 1.0;
    }
}