#version 430 core

layout(location = 0) uniform mat4  u_ModelViewMatrix;
layout(location = 1) uniform mat4  u_ProjectionMatrix;
layout(location = 2) uniform mat4  u_LightViewMatrix;

in vec4 a_Vertex;
in vec3 a_Normal;
in vec2 a_TexCoord;

out vec3 v_Normal;
out vec2 v_TexCoord;
out vec4 v_Vertex;
out vec4 v_LightSpacePos;

void main(void) {
    v_LightSpacePos = u_LightViewMatrix * a_Vertex;

    v_Vertex   = u_ModelViewMatrix * a_Vertex;
    v_Normal   = mat3(u_ModelViewMatrix) * a_Normal;
    v_TexCoord = a_TexCoord;

    gl_Position = u_ProjectionMatrix * v_Vertex;
}
