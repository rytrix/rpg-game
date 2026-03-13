#version 460 core
// Vertex Begin
layout (location = 0) in vec3 inPos;

#ifdef ENABLE_BONES
layout (location = 4) in ivec4 inBoneIDs;
layout (location = 5) in vec4 inBoneWeights;
uniform mat4 final_bone_matrices[MAX_BONES];
#endif

layout(binding = 1, std430) readonly buffer ssbo0 {
    mat4 models[];
};

uniform mat4 light_space_matrix;

out vec4 FragPos;

void main()
{
#ifdef ENABLE_BONES
    mat4 bone_transform = final_bone_matrices[inBoneIDs[0]] * inBoneWeights[0];
    for (int i = 1; i < BONES_PER_VERTEX; i++) {
        bone_transform += final_bone_matrices[inBoneIDs[i]] * inBoneWeights[i];
    }
#endif
    mat4 model = models[gl_InstanceID];
#ifdef ENABLE_BONES
    FragPos = model * (bone_transform * vec4(inPos, 1.0));
    gl_Position = light_space_matrix * FragPos;
#else
    FragPos = model * vec4(inPos, 1.0);
    gl_Position = light_space_matrix * FragPos;
#endif
}
// Vertex End

#version 460 core
// Fragment Begin
in vec4 FragPos;

uniform vec3 light_pos;
uniform float far_plane;

void main()
{
    float light_distance = length(FragPos.xyz - light_pos);

    // map to [0;1] range
    light_distance = light_distance / far_plane;

    gl_FragDepth = light_distance;
}
// Fragment End
