#version 460 core
// Vertex Begin
layout (location = 0) in vec3 inPos;

#ifdef ENABLE_BONES
layout (location = 4) in ivec4 inBoneIDs;
layout (location = 5) in vec4 inBoneWeights;
layout(binding = 2, std430) readonly buffer ssbo1 {
    mat4 final_bone_matrices[];
};
#endif

#ifdef MODEL_UNIFORM
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
#ifdef ENABLE_BONES
    mat4 bone_transform = final_bone_matrices[gl_InstanceID * MAX_BONES + inBoneIDs[0]] * inBoneWeights[0];
    for (int i = 1; i < BONES_PER_VERTEX; i++) {
        bone_transform += final_bone_matrices[gl_InstanceID * MAX_BONES + inBoneIDs[i]] * inBoneWeights[i];
    }
#endif

#ifdef SSBO0
    mat4 model = models[gl_InstanceID];
#endif

#ifdef ENABLE_BONES
    vec4 world_pos = model * (bone_transform * vec4(inPos, 1.0));
    // Normal and Tangent should be different because of bones
    // mat3 transposed_model = mat3(transpose(inverse(model)));
    // mat3 transposed_bone_transform = mat3(transpose(inverse(bone_transform)));
    // Normal = transposed_model * (transposed_bone_transform * inNormal);
    // Tangent = transposed_model * (transposed_bone_transform * inTangent);
#else
    vec4 world_pos = model * vec4(inPos, 1.0);
    // mat3 transposed_model = mat3(transpose(inverse(model)));
    // Normal = transposed_model * inNormal;
    // Tangent = transposed_model * inTangent;
#endif
    gl_Position = proj * view * world_pos;
}
// Vertex End

#version 460
// Fragment Begin

out vec4 FragColor;

uniform vec4 u_color;

void main() {
    FragColor = u_color;
}

// Fragment End