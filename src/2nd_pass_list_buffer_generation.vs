#version 430 core

#ifdef USER_TEST
#endif

uniform int   u_UserVariableInt;
uniform float u_UserVariableFloat;

layout (location = 0) in vec3 a_Vertex;


void main(void) {
    gl_Position     = vec4(a_Vertex.xy, 0.0f, 1.0f);
}
