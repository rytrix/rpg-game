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
    mat4 light_space_matrix;
    sampler2D shadow_map;
};
uniform DirectionalLight u_directional_light;

float shadow_calculation(DirectionalLight light, vec3 frag_pos, float bias)
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

    float shadow_bias = max(0.05 * (1.0 - dot(normal, light_dir)), 0.005);  
    float shadow = shadow_calculation(light, frag_pos, shadow_bias);

    return (ambient + (1.0 - shadow) * (diffuse + specular)) * albedo;
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