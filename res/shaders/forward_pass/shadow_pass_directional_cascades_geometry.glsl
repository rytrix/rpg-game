#version 460 core
// Vertex Begin
layout (location = 0) in vec3 inPos;

layout(binding = 1, std430) readonly buffer ssbo0 {
    mat4 models[];
};

void main()
{
    mat4 model = models[gl_InstanceID];
    gl_Position = model * vec4(inPos, 1.0);
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