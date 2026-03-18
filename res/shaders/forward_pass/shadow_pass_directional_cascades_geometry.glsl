#version 460 core
// Vertex Begin
layout (location = 0) in vec3 inPos;
#ifdef ENABLE_BONES
layout (location = 4) in int[BONES_PER_VERTEX] inBoneIDs;
layout (location = 5) in float[BONES_PER_VERTEX] inBoneWeights;
layout(binding = 2, std430) readonly buffer ssbo1 {
    mat4 final_bone_matrices[];
};
#endif

layout(binding = 1, std430) readonly buffer ssbo0 {
    mat4 models[];
};

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
    gl_Position = model * (bone_transform * vec4(inPos, 1.0));
#else
    gl_Position = model * vec4(inPos, 1.0);
#endif
}
// Vertex End

#version 460 core
#define CASCADE_COUNT 4
// Geometry Begin
layout(triangles, invocations = CASCADE_COUNT) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 light_space_matrices[CASCADE_COUNT];

void main()
{          
    for (int i = 0; i < 3; ++i)
    {
        gl_Position = 
            light_space_matrices[gl_InvocationID] * gl_in[i].gl_Position;
        gl_Layer = gl_InvocationID;
        EmitVertex();
    }
    EndPrimitive();
}
// Geometry End

#version 460 core
// Fragment Begin
void main()
{             
}
// Fragment End