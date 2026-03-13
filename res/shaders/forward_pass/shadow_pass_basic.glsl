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
    gl_Position = light_space_matrix * model * (bone_transform * vec4(inPos, 1.0));
#else
    gl_Position = light_space_matrix * model * vec4(inPos, 1.0);
#endif
}
// Vertex End

#version 460 core
// Fragment Begin
void main()
{
}
// Fragment End