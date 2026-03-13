#version 460 core

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

struct PointLight {
    vec3 position;
    vec3 color;
};

struct DirectionalLight {
    vec3 direction;
    vec3 color;
};

const float PI = 3.14159265359;

float distribution_ggx(float NdotH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
	
    float denom = (NdotH * NdotH * (a2 - 1.0) + 1.0);
    denom = max(PI * denom * denom, 0.0000001);
	
    return a2 / denom;
}

float geometry_smith(float NdotV, float NdotL, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    float ggx1 = NdotV / (NdotV * (1.0 - k) + k);
    float ggx2 = NdotL / (NdotL * (1.0 - k) + k);
    return ggx1 * ggx2;
}

vec3 fresnel_schlick(float NdotV, vec3 base_reflectivity)
{
    return base_reflectivity + (1.0 - base_reflectivity) * pow(clamp(1.0 - NdotV, 0.0, 1.0), 5.0);
} 

vec3 pbr_base(
    vec3 albedo,
    float roughness,
    float metallic,
    vec3 base_reflectivity,
    vec3 radiance,
    vec3 N,
    vec3 V,
    vec3 L,
    vec3 H)
{
    float NdotV = max(dot(N, V), 0.0000001);
    float NdotL = max(dot(N, L), 0.0000001);
    float HdotV = max(dot(H, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);

    float D = distribution_ggx(NdotH, roughness);
    float G = geometry_smith(NdotV, NdotL, roughness);
    vec3 F = fresnel_schlick(NdotV, base_reflectivity);

    vec3 specular = D * G * F;
    specular /= 4.0 * NdotV * NdotL;

    vec3 kD = vec3(1.0) - F;
    kD *= 1.0 - metallic;

    return (kD * albedo / PI + specular) * radiance * NdotL;
}

vec3 pbr_directional(
    DirectionalLight light, 
    vec3 albedo,
    float roughness,
    float metallic,
    vec3 N,
    vec3 V)
{
    vec3 base_reflectivity = mix(vec3(0.04), albedo, metallic);

    vec3 L = normalize(-light.direction);
    vec3 H = normalize(V + L);
    
    vec3 radiance = light.color;

    return pbr_base(albedo, roughness, metallic, base_reflectivity, radiance, N, V, L, H);
}

vec3 pbr_point(
    PointLight light, 
    vec3 albedo,
    float roughness,
    float metallic,
    vec3 N,
    vec3 V)
{
    vec3 base_reflectivity = mix(vec3(0.04), albedo, metallic);

    vec3 L = normalize(light.position - FragPos);
    vec3 H = normalize(V + L);
    
    float light_distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light_distance * light_distance);
    vec3 radiance = light.color * attenuation;

    return pbr_base(albedo, roughness, metallic, base_reflectivity, radiance, N, V, L, H);
}

uniform sampler2D diffuse;
uniform sampler2D metallic_roughness;
uniform sampler2D specular;
uniform vec3 view_pos;

void main()
{
    // Mesh
    vec3 albedo = texture(diffuse, TexCoords).rgb;
    vec4 metallic_roughness_texture = texture(metallic_roughness, TexCoords);

    float roughness = metallic_roughness_texture.g;
    float metallic = metallic_roughness_texture.b;
    float ao = 1.0;

    vec3 normal = normalize(Normal);
    vec3 view = normalize(view_pos - FragPos);
    
    vec3 lo = vec3(0.0);
    PointLight point;
    point.position = vec3(8.0, 6.0, -12.0);
    point.color = vec3(200.0, 100.0, 100.0);
    lo = pbr_point(point, albedo, roughness, metallic, normal, view);

    point.position = vec3(-6.0, 6.0, 6.0);
    point.color = vec3(100.0);
    lo += pbr_point(point, albedo, roughness, metallic, normal, view);

    DirectionalLight directional;
    directional.direction = vec3(-0.2, -0.5, 0.3);
    directional.color = vec3(1.0, 1.0, 2.0);
    lo += pbr_directional(directional, albedo, roughness, metallic, normal, view);

    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
}