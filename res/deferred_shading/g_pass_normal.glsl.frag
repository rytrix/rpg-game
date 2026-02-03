#version 460 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D diffuse;
uniform sampler2D specular;

void main()
{
    gPosition = FragPos;
    gNormal = normalize(Normal);
    gAlbedo.rgb = texture(diffuse, TexCoords).rgb;
    gAlbedo.a = texture(specular, TexCoords).r;
}