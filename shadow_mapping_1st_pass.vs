#version 430 core

#ifdef USER_TEST
#endif

layout(location = 0) uniform mat4  u_ModelViewMatrix;
layout (location = 1) uniform mat4  u_ProjectionMatrix;
uniform int   u_UserVariableInt;
uniform float u_UserVariableFloat;

layout (location = 0) in vec4 a_Vertex;

out vec4 v_LightSpacePos;

void main(void) {
    v_LightSpacePos = u_ModelViewMatrix * a_Vertex;
    gl_Position     = u_ProjectionMatrix * v_LightSpacePos;
}
