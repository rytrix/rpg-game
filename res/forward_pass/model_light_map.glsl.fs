#version 460 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

struct DirectionalLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform DirectionalLight u_directional_light;

struct PointLight {
    vec3 pos;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    // https://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation
    float constant;
    float linear;
    float quadratic;
};
uniform PointLight u_point_light;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};
uniform Material u_material;

uniform vec3 view_pos;

float shadow_calculation(vec4 frag_pos_light_space) 
{
    return 0.0;
}

vec4 directional_light(DirectionalLight light, Material material)
{
    vec3 norm = normalize(Normal);
    vec3 light_dir = normalize(-light.direction);

    vec4 texture_diffuse = texture(material.diffuse, TexCoords);
    vec4 texture_specular = texture(material.specular, TexCoords);

    vec3 ambient = texture_diffuse.xyz * light.ambient;

    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = (diff * texture_diffuse.xyz) * light.diffuse;

    vec3 view_dir = normalize(view_pos - FragPos);
    vec3 reflect_dir = reflect(-light_dir, norm);

    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular = (texture_specular.xyz * spec) * light.specular;

    return vec4(ambient + diffuse + specular, texture_diffuse.w);
}

vec4 point_light(PointLight light, Material material)
{
    float light_distance = length(light.pos - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * light_distance + light.quadratic * (light_distance * light_distance));  

    vec4 texture_diffuse = texture(material.diffuse, TexCoords);
    vec4 texture_specular = texture(material.specular, TexCoords);

    vec3 ambient = texture_diffuse.xyz * light.ambient;

    vec3 norm = normalize(Normal);
    vec3 light_dir = normalize(light.pos - FragPos);

    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = (diff * texture_diffuse.xyz) * light.diffuse;

    vec3 view_dir = normalize(view_pos - FragPos);
    vec3 reflect_dir = reflect(-light_dir, norm);

    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular = (texture_specular.xyz * spec) * light.specular;

    return vec4(ambient * attenuation + diffuse * attenuation + specular * attenuation, texture_diffuse.w);
}

void main()
{    

    FragColor = directional_light(u_directional_light, u_material);
    FragColor += point_light(u_point_light, u_material);
}
