constexpr const char* PHONG_LIGHTING_SHADER_CODE = R"(
struct DirectionalLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    mat4 light_space_matrix;
};

struct DirectionalLightShadow {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    mat4 light_space_matrix;
    sampler2D shadow_map;
};

struct PointLight {
    vec3 pos;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

struct PointLightShadow {
    vec3 pos;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;

    samplerCube shadow_map;
    float far_plane;
};

float shadow_calculation(DirectionalLightShadow light, vec3 frag_pos, float bias)
{
    vec4 light_space_frag_pos = light.light_space_matrix * vec4(frag_pos, 1.0);

    vec3 proj_coords = light_space_frag_pos.xyz / light_space_frag_pos.w;

    // transform to [0,1] range
    proj_coords = proj_coords * 0.5 + 0.5;

    float closest_depth = texture(light.shadow_map, proj_coords.xy).x;

    float current_depth = proj_coords.z;

    float shadow = 0.0;
    vec2 texel_size = 1.0 / textureSize(light.shadow_map, 0);
    for(int x = -1; x <= 1; x++)
    {
        for(int y = -1; y <= 1; y++)
        {
            float pcfDepth = texture(light.shadow_map, proj_coords.xy + vec2(x, y) * texel_size).r; 
            shadow += current_depth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;

    // float shadow = current_depth - bias > closest_depth ? 1.0 : 0.0;

    if (proj_coords.z > 1.0) {
        shadow = 0.0;
    }

    return shadow;
}

vec3 sample_offset_directions[20] = vec3[]
(
    vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
    vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
    vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
    vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
    vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

float shadow_calculation_cubemap(PointLightShadow light, vec3 frag_pos, float bias)
{
    vec3 frag_to_light = frag_pos - light.pos;
    float current_depth = length(frag_to_light);

    // pcf 2
    float shadow = 0.0;
    // float bias   = 0.15;
    int samples  = 20;
    float view_distance = length(light.pos - frag_pos);
    float disk_radius = (1.0 + (view_distance / light.far_plane)) / 25.0;  

    for(int i = 0; i < samples; ++i)
    {
        float closest_depth = texture(light.shadow_map, frag_to_light + sample_offset_directions[i] * disk_radius).r;
        closest_depth *= light.far_plane;   // undo mapping [0;1]
        if(current_depth - bias > closest_depth)
            shadow += 1.0;
    }
    shadow /= float(samples);  

    return shadow;
}

vec3 directional_light(DirectionalLight light, vec3 albedo, float in_specular, vec3 normal, vec3 view_pos, vec3 frag_pos, float shininess)
{
    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(-light.direction);

    vec3 ambient = light.ambient;

    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = (diff) * light.diffuse;

    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);

    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), shininess);
    vec3 specular = (vec3(in_specular) * spec) * light.specular;

    return (ambient + diffuse + specular) * albedo;
}

vec3 directional_light_shadow(DirectionalLightShadow light, vec3 albedo, float in_specular, vec3 normal, vec3 view_pos, vec3 frag_pos, float shininess)
{
    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(-light.direction);

    vec3 ambient = light.ambient;

    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = (diff) * light.diffuse;

    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);

    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), shininess);
    vec3 specular = (vec3(in_specular) * spec) * light.specular;

    // float shadow_bias = max(0.05 * (1.0 - dot(normal, light_dir)), 0.005);  
    float shadow_bias = 0.05;
    float shadow = shadow_calculation(light, frag_pos, shadow_bias);
    // float shadow = 0.0;

    return (ambient + (1.0 - shadow) * (diffuse + specular)) * albedo;
}

vec3 point_light(PointLight light, vec3 albedo, float in_specular, vec3 normal, vec3 view_pos, vec3 frag_pos, float shininess)
{
    float light_distance = length(light.pos - frag_pos);
    float attenuation = 1.0 / (light.constant + light.linear * light_distance + light.quadratic * (light_distance * light_distance));

    vec3 ambient = light.ambient;

    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(light.pos - frag_pos);

    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * light.diffuse;

    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);

    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), shininess);
    vec3 specular = (in_specular * spec) * light.specular;

    return (ambient + diffuse + specular) * albedo;
}

vec3 point_light_shadow(PointLightShadow light, vec3 albedo, float in_specular, vec3 normal, vec3 view_pos, vec3 frag_pos, float shininess)
{
    float light_distance = length(light.pos - frag_pos);
    float attenuation = 1.0 / (light.constant + light.linear * light_distance + light.quadratic * (light_distance * light_distance));

    vec3 ambient = light.ambient;

    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(light.pos - frag_pos);

    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * light.diffuse;

    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);

    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), shininess);
    vec3 specular = (in_specular * spec) * light.specular;

    // float shadow_bias = max(0.05 * (1.0 - dot(normal, light_dir)), 0.005);
    float shadow_bias = 0.15;
    float shadow = shadow_calculation_cubemap(light, frag_pos, shadow_bias);

    return (ambient + (1.0 - shadow) * (diffuse + specular)) * albedo;
})";

constexpr const char* PBR_LIGHTING_SHADER_CODE = R"(
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
)";

constexpr std::string get_pbr_forward_pass_indirect(const std::string& light_uniforms, const std::string& light_functions)
{
    std::string shader_source = std::format(
        R"(
#version 460 core
#extension GL_ARB_bindless_texture : require

out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;
in vec3 Tangent;
in flat int DrawID;

// Lighting code
{}

layout(binding = 2, std430) readonly buffer ssbo1 {{
    sampler2D tex_diffuse[];
}};

layout(binding = 3, std430) readonly buffer ssbo2 {{
    sampler2D tex_metallic_roughness[];
}};

layout(binding = 4, std430) readonly buffer ssbo3 {{
    sampler2D tex_normals[];
}};

uniform int diffuse_max_textures;
uniform int metallic_roughness_max_textures;
uniform int normals_max_textures;

uniform vec3 view_position;

vec3 calc_bumped_normal()
{{
    vec3 normal = normalize(Normal);
    vec3 tangent = normalize(Tangent);
    tangent = normalize(tangent - dot(tangent, normal) * normal);
    vec3 bitangent = cross(tangent, normal);

    vec3 bump_map_normal = texture(tex_normals[DrawID], TexCoords).xyz;
    bump_map_normal = 2.0 * bump_map_normal - vec3(1.0, 1.0, 1.0);

    vec3 new_normal;
    mat3 TBN = mat3(tangent, bitangent, normal);
    new_normal = TBN * bump_map_normal;
    new_normal = normalize(new_normal);

    return new_normal;
}}

// Light uniforms
{}

void main() {{
    // vec3 normal = normalize(Normal);
    vec3 normal = calc_bumped_normal();

    vec3 albedo = vec3(1.0);
    float metallic = 0.0;
    float roughness = 0.0;
    float specular = 0.0;
    float ao = 1.0;
    vec3 view = normalize(view_position - FragPos);

    vec4 diffuse = texture(tex_diffuse[DrawID], TexCoords);
    if (diffuse.a < 0.01) {{
        discard;
    }}
    albedo.rgb = diffuse.rgb;

    vec4 metallic_roughness = texture(tex_metallic_roughness[DrawID], TexCoords);
    ao = metallic_roughness.r;
    metallic = metallic_roughness.b;
    roughness = metallic_roughness.g;

    vec3 lo = vec3(0.0);

    {}

    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, diffuse.a);
}})",
        PBR_LIGHTING_SHADER_CODE,
        light_uniforms,
        light_functions);

    return shader_source;
}

constexpr std::string get_pbr_forward_pass_normal(const std::string& light_uniforms, const std::string& light_functions)
{
    std::string shader_source = std::format(
        R"(
#version 460 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;
in vec3 Tangent;

// Lighting code
{}

uniform sampler2D tex_diffuse;
uniform sampler2D tex_metallic_roughness;
uniform sampler2D tex_normals;

uniform vec3 view_position;

vec3 calc_bumped_normal()
{{
    vec3 normal = normalize(Normal);
    vec3 tangent = normalize(Tangent);
    tangent = normalize(tangent - dot(tangent, normal) * normal);
    vec3 bitangent = cross(tangent, normal);

    vec3 bump_map_normal = texture(tex_normals, TexCoords).xyz;
    bump_map_normal = 2.0 * bump_map_normal - vec3(1.0, 1.0, 1.0);

    vec3 new_normal;
    mat3 TBN = mat3(tangent, bitangent, normal);
    new_normal = TBN * bump_map_normal;
    new_normal = normalize(new_normal);

    return new_normal;
}}

// Light uniforms
{}

void main() {{
    // vec3 normal = normalize(Normal);
    vec3 normal = calc_bumped_normal();
    vec3 albedo = vec3(1.0);
    float metallic = 0.0;
    float roughness = 0.0;
    float specular = 0.0;
    float ao = 1.0;
    vec3 view = normalize(view_position - FragPos);

    vec4 diffuse = texture(tex_diffuse, TexCoords);
    if (diffuse.a < 0.01) {{
        discard;
    }}
    albedo.rgb = diffuse.rgb;
    vec4 metallic_roughness = texture(tex_metallic_roughness, TexCoords);
    ao = metallic_roughness.r;
    metallic = metallic_roughness.b;
    roughness = metallic_roughness.g;

    vec3 lo = vec3(0.0);

    {}

    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, diffuse.a);
}})",
        PBR_LIGHTING_SHADER_CODE,
        light_uniforms,
        light_functions);

    return shader_source;
}

constexpr std::string get_phong_forward_pass_indirect(const std::string& light_uniforms, const std::string& light_functions)
{
    std::string shader_source = std::format(
        R"(
#version 460 core
#extension GL_ARB_bindless_texture : require

out vec4 FragColor;

// Lighting code
{}

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;
in flat int DrawID;

layout(binding = 2, std430) readonly buffer ssbo1 {{
    sampler2D diffuse[];
}};

layout(binding = 3, std430) readonly buffer ssbo2 {{
    sampler2D metallic_roughness[];
}};

layout(binding = 4, std430) readonly buffer ssbo3 {{
    sampler2D specular[];
}};

uniform int diffuse_max_textures;
uniform int metallic_roughness_max_textures;
uniform int specular_max_textures;
uniform vec3 view_position;

// Light uniforms
{}

void main() {{
    vec3 Normal = normalize(Normal);
    vec3 Albedo = vec3(0.0);
    float Specular = 0.0;
    if (DrawID < diffuse_max_textures) {{
        Albedo.rgb = texture(diffuse[DrawID], TexCoords).rgb;
    }} else {{
        Albedo.rgb = vec3(0.0);
    }}

    if (DrawID < specular_max_textures) {{
        Specular = texture(specular[DrawID], TexCoords).r;
    }} else {{
        Specular = 0.0;
    }}
    
    FragColor = vec4(0.0);
    {}
    // FragColor = vec4(Albedo, 1.0);
}})",
        PHONG_LIGHTING_SHADER_CODE,
        light_uniforms,
        light_functions);

    return shader_source;
}

constexpr std::string get_phong_forward_pass_normal(const std::string& light_uniforms, const std::string& light_functions)
{
    std::string shader_source = std::format(
        R"(
#version 460 core

out vec4 FragColor;

// Lighting code
{}

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D diffuse;
uniform sampler2D specular;
uniform vec3 view_position;

// Light uniforms
{}

void main() {{
    vec3 Normal = normalize(Normal);
    vec3 Albedo = texture(diffuse, TexCoords).rgb;
    float Specular = texture(specular, TexCoords).r;
    
    FragColor = vec4(vec3(0.0), 1.0);
    {}
}})",
        PHONG_LIGHTING_SHADER_CODE,
        light_uniforms,
        light_functions);

    return shader_source;
}

constexpr std::string get_deferred_pass(const std::string& light_uniforms, const std::string& light_functions)
{
    std::string shader_source_frag = std::format(
        R"(
#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

// Lighting code
{}

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform vec3 view_position;

// Light uniforms
{}

void main() {{
    vec3 FragPos = texture(gPosition, TexCoords).xyz;
    vec3 Normal = texture(gNormal, TexCoords).xyz;
    vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;
    FragColor = vec4(0.0);
    {}
    // FragColor = vec4(Albedo, 1.0);
}})",
        PHONG_LIGHTING_SHADER_CODE,
        light_uniforms,
        light_functions);

    return shader_source_frag;
}