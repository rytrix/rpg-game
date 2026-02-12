#version 460 core

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

const float PI = 3.14159265359;

struct PointLight {
    vec3 position;
    vec3 color;
};

struct DirectionalLight {
    vec3 direction;
    vec3 color;
};

// GGX/Trowbridge-Reitz Normal Distribution Function
float normal_distribution(float alpha, vec3 normal, vec3 halfway)
{
    float numerator = pow(alpha, 2.0);

    float normal_dot_halfway = max(dot(normal, halfway), 0.0);
    float denominator = PI * pow(pow(normal_dot_halfway, 2.0) * (pow(alpha, 2.0) - 1.0) + 1.0, 2.0);
    denominator = max(denominator, 0.000001);

    return numerator / denominator;
}

// Schlick-Beckmann Geometry Shadowing Function
float G1(float alpha, vec3 normal, vec3 x)
{
    float numerator = max(dot(normal, x), 0.0);

    float k = alpha / 2.0;
    float denominator = max(dot(normal, x), 0.0) * (1.0 - k) + k;
    denominator = max(denominator, 0.000001);

    return numerator / denominator;
}

// Smith Model
float geometry_shadowing(float alpha, vec3 normal, vec3 view, vec3 light)
{
    return G1(alpha, normal, view) * G1(alpha, normal, light);
}

// Fresnel-Schlick Function
vec3 fresnel(vec3 F0, vec3 view, vec3 halfway)
{
    return F0 + (vec3(1.0) - F0) * pow(1.0 - max(dot(view, halfway), 0.0), 5.0); 
}

// Learnopengl functions
float distribution_ggx(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float geometry_schlick_ggx(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float geometry_smith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = geometry_schlick_ggx(NdotV, roughness);
    float ggx1  = geometry_schlick_ggx(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 fresnel_schlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
} 

float distribution_ggx2(float NdotH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
	
    float denom = (NdotH * NdotH * (a2 - 1.0) + 1.0);
    denom = max(PI * denom * denom, 0.0000001);
	
    return a2 / denom;
}

float geometry_smith2(float NdotV, float NdotL, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    float ggx1 = NdotV / (NdotV * (1.0 - k) + k);
    float ggx2 = NdotL / (NdotL * (1.0 - k) + k);
    return ggx1 * ggx2;
}

vec3 fresnel_schlick2(float NdotV, vec3 base_reflectivity)
{
    return base_reflectivity + (1.0 - base_reflectivity) * pow(clamp(1.0 - NdotV, 0.0, 1.0), 5.0);
} 

vec3 pbr_point(
    PointLight point_light, 
    vec3 albedo,
    vec3 emissive,
    float roughness,
    float metallic,
    vec3 normal,
    vec3 view)
{
    // Directional
    // vec3 light = normalize(Light.position);

    // Point/Spotlight
    vec3 light = normalize(point_light.position - FragPos);

    vec3 halfway = normalize(view + light);

    // is there something else I have to do for f0?
    // base reflectance is not used..
    vec3 f0 = vec3(0.04);
    f0 = mix(f0, albedo, metallic);
    // vec3 f0 = albedo;

    vec3 k_specular = fresnel(f0, view, halfway);
    vec3 k_diffuse = (vec3(1.0) - k_specular) * (1.0 - metallic);

    vec3 lambert = albedo / PI;

    vec3 cook_torrance_numerator 
        = normal_distribution(roughness, normal, halfway)
        * geometry_shadowing(roughness, normal, view, light)
        * fresnel(f0, view, halfway);

    float cook_torrance_denominator =
        4.0 * max(dot(view, normal), 0.0) * max(dot(light, normal), 0.0);
    cook_torrance_denominator = max(cook_torrance_denominator, 0.000001);

    vec3 cook_torrance = cook_torrance_numerator / cook_torrance_denominator;

    vec3 BRDF = k_diffuse * lambert + cook_torrance;
    vec3 outgoing_light = emissive + BRDF * point_light.color; // * max(dot(light, normal), 0.0);

    return outgoing_light;
}

vec3 pbr_point2(
    PointLight point_light, 
    vec3 albedo,
    float roughness,
    float metallic,
    vec3 normal,
    vec3 view)
{
    vec3 f0 = vec3(0.04);
    f0 = mix(f0, albedo, metallic);

    // Directional
    // vec3 light = normalize(Light.position);

    // Point/Spotlight
    vec3 light = normalize(point_light.position - FragPos);
    vec3 halfway = normalize(view + light);

    float light_distance = length(point_light.position - FragPos);
    // float attenuation = 1.0 / light_distance;
    float attenuation = 1.0 / (light_distance * light_distance);
    vec3 radiance = point_light.color * attenuation;

    float NDF = distribution_ggx(normal, halfway, roughness);       
    float G = geometry_smith(normal, view, light, roughness);  
    vec3 F = fresnel_schlick(max(dot(halfway, view), 0.0), f0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, view), 0.0) * max(dot(normal, light), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;  

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
  
    kD *= 1.0 - metallic;	

    float NdotL = max(dot(normal, light), 0.0);        
    vec3 lo = (kD * albedo / PI + specular) * radiance * NdotL;

    return lo;
}

vec3 pbr_point3(
    PointLight point_light, 
    vec3 albedo,
    float roughness,
    float metallic,
    vec3 N,
    vec3 V)
{
    vec3 base_reflectivity = mix(vec3(0.04), albedo, metallic);

    // Directional
    // vec3 L = normalize(Light.position);

    // Point/Spotlight
    vec3 L = normalize(point_light.position - FragPos);
    vec3 H = normalize(V + L);
    
    float light_distance = length(point_light.position - FragPos);
    float attenuation = 1.0 / (light_distance * light_distance);
    vec3 radiance = point_light.color * attenuation;

    float NdotV = max(dot(N, V), 0.0000001);
    float NdotL = max(dot(N, L), 0.0000001);
    float HdotV = max(dot(H, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);

    float D = distribution_ggx2(NdotH, roughness);
    float G = geometry_smith2(NdotV, NdotL, roughness);
    vec3 F = fresnel_schlick2(NdotV, base_reflectivity);

    vec3 specular = D * G * F;
    specular /= 4.0 * NdotV * NdotL;

    vec3 kD = vec3(1.0) - F;
    kD *= 1.0 - metallic;

    return (kD * albedo / PI + specular) * radiance * NdotL;
}

uniform sampler2D diffuse;
uniform sampler2D metallic_roughness;
uniform sampler2D specular;
uniform vec3 view_pos;

// start
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
    lo = pbr_point3(point, albedo, roughness, metallic, normal, view);

    point.position = vec3(-6.0, 6.0, 6.0);
    point.color = vec3(100.0);
    lo += pbr_point3(point, albedo, roughness, metallic, normal, view);

    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
}
// end