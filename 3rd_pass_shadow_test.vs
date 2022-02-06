#version 430 core

layout(location = 0) uniform mat4  u_ModelViewMatrix;
layout (location = 1) uniform mat4  u_ProjectionMatrix;

layout (location = 0) in vec4 a_Vertex;

out Data {
    smooth vec4 v_LightSpacePos;
    flat vec4 plane;
    flat vec3 triangle_vertices[3];
} Out;

void main(void) {
    Out.v_LightSpacePos = u_ModelViewMatrix * a_Vertex;
    gl_Position     = u_ProjectionMatrix * Out.v_LightSpacePos;
}
