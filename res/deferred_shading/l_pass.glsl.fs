#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

uniform vec3 view_position;

struct DirectionalLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform DirectionalLight u_directional_light;

vec3 directional_light(DirectionalLight light, vec3 albedo, float in_specular, vec3 normal, vec3 view_pos, vec3 frag_pos, float shininess)
{
    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(-light.direction);

    vec3 ambient = albedo * light.ambient;

    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = (diff * albedo) * light.diffuse;

    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);

    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), shininess);
    vec3 specular = (vec3(in_specular) * spec) * light.specular;

    return ambient + diffuse + specular;
}

void main()
{
    vec3 FragPos = texture(gPosition, TexCoords).xyz;
    vec3 Normal = texture(gNormal, TexCoords).xyz;
    vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;

    FragColor = vec4(directional_light(u_directional_light, Albedo, Specular, Normal, view_position, FragPos, 32.0), 1.0);

    // FragColor = vec4(Albedo, 1.0);
}