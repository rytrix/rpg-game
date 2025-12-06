#version 460 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

struct Light {
    vec3 pos;
    float ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform Light light;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};
uniform Material material;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform vec3 view_pos;

// vec3 light.color = vec3(1.0, 1.0, 0.268);
// float ambient_strength = 0.1;
// float specular_strength = 0.5;

// vec3 light.pos = vec3(8.0, 8.0, 8.0);

void main()
{    
    vec3 ambient = material.ambient * light.ambient;

    vec3 norm = normalize(Normal);
    vec3 light_dir = normalize(light.pos - FragPos);

    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = (diff * material.diffuse) * light.diffuse;

    vec3 view_dir = normalize(view_pos - FragPos);
    vec3 reflect_dir = reflect(-light_dir, norm);

    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular = (material.specular * spec) * light.specular;

    vec4 color = texture(texture_diffuse1, TexCoords);
    color = vec4((ambient + diffuse + specular) * color.xyz, color.w);

    FragColor = color;
}
