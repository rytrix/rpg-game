#version 460 core
// Vertex Begin
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoords;
layout (location = 3) in vec3 inTangent;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;
out vec3 Tangent;
out flat int DrawID;

#ifdef ModelUniform
uniform mat4 model;
#endif
uniform mat4 view;
uniform mat4 proj;

#ifdef SSBO0
layout(binding = 1, std430) readonly buffer ssbo0 {
    mat4 models[];
};
#endif

void main()
{
#ifdef SSBO0
    mat4 model = models[gl_InstanceID];
#endif
    vec4 world_pos = model * vec4(inPos, 1.0);
    TexCoords = inTexCoords;

    mat3 transposed_model = mat3(transpose(inverse(model)));
    Normal = transposed_model * inNormal;
    Tangent = transposed_model * inTangent;

    FragPos = world_pos.xyz;

    DrawID = gl_DrawID;

    gl_Position = proj * view * world_pos;
}
// Vertex End

#version 460 core
// Fragment Begin
#ifdef BindlessTextures
#extension GL_ARB_bindless_texture : require
#endif

out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;
in vec3 Tangent;
in flat int DrawID;

struct PointLight {
    vec3 position;
    vec3 color;
};

struct DirectionalLight {
    vec3 direction;
    vec3 color;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float inner_cutoff;
    float outer_cutoff;
};

#ifdef UniformTextures
uniform sampler2D tex_diffuse;
uniform sampler2D tex_metallic_roughness;
uniform sampler2D tex_normals;
#endif

#ifdef BindlessTextures
layout(binding = 2, std430) readonly buffer ssbo1 {
    sampler2D tex_diffuse[];
};

layout(binding = 3, std430) readonly buffer ssbo2 {
    sampler2D tex_metallic_roughness[];
};

layout(binding = 4, std430) readonly buffer ssbo3 {
    sampler2D tex_normals[];
};

uniform int diffuse_max_textures;
uniform int metallic_roughness_max_textures;
uniform int normals_max_textures;
#endif

uniform vec3 view_position;

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

vec3 pbr_spot(
    SpotLight light, 
    vec3 albedo,
    float roughness,
    float metallic,
    vec3 N,
    vec3 V)
{
    vec3 light_dir = normalize(light.position - FragPos);
    float theta = dot(light_dir, normalize(-light.direction));
    
    float epsilon = light.inner_cutoff - light.outer_cutoff;
    float intensity = clamp((theta - light.outer_cutoff) / epsilon, 0.0, 1.0);

    vec3 base_reflectivity = mix(vec3(0.04), albedo, metallic);

    vec3 L = normalize(light.position - FragPos);
    vec3 H = normalize(V + L);
    
    float light_distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light_distance * light_distance);
    vec3 radiance = light.color * attenuation;

    return pbr_base(albedo, roughness, metallic, base_reflectivity, radiance, N, V, L, H) * intensity;
}

vec3 calc_bumped_normal(vec3 bump_map_normal)
{
    vec3 normal = normalize(Normal);
    vec3 tangent = normalize(Tangent);
    tangent = normalize(tangent - dot(tangent, normal) * normal);
    vec3 bitangent = cross(tangent, normal);

    // vec3 bump_map_normal = texture(tex_normals, TexCoords).xyz;
    bump_map_normal = 2.0 * bump_map_normal - vec3(1.0, 1.0, 1.0);

    vec3 new_normal;
    mat3 TBN = mat3(tangent, bitangent, normal);
    new_normal = TBN * bump_map_normal;
    new_normal = normalize(new_normal);

    return new_normal;
}

// Light Uniforms Begin
// Light Uniforms End

void main() {
#ifdef UniformTextures
    vec3 bump_map_normal = texture(tex_normals, TexCoords).xyz;
    vec4 diffuse = texture(tex_diffuse, TexCoords);
    vec4 metallic_roughness = texture(tex_metallic_roughness, TexCoords);
#endif

#ifdef BindlessTextures
    vec3 bump_map_normal = texture(tex_normals[DrawID], TexCoords).xyz;
    vec4 diffuse = texture(tex_diffuse[DrawID], TexCoords);
    vec4 metallic_roughness = texture(tex_metallic_roughness[DrawID], TexCoords);
#endif

    // vec3 normal = normalize(Normal);
    vec3 normal = calc_bumped_normal(bump_map_normal);
    vec3 albedo = vec3(1.0);
    float metallic = 0.0;
    float roughness = 0.0;
    float specular = 0.0;
    float ao = 1.0;
    vec3 view = normalize(view_position - FragPos);

    if (diffuse.a < 0.01) {
        discard;
    }
    albedo.rgb = diffuse.rgb;
    ao = metallic_roughness.r;
    metallic = metallic_roughness.b;
    roughness = metallic_roughness.g;

    vec3 lo = vec3(0.0);

// LO Functions Begin
// LO Functions End

    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, diffuse.a);
}
// Fragment End