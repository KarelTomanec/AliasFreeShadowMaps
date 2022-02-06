#version 430 core

#ifdef USER_TEST
#endif

uniform int   u_UserVariableInt;
uniform float u_UserVariableFloat;

layout (location = 0) out vec4 FragColor;

in vec4 v_LightSpacePos;

void main(void) {
    // TODO: Compute vertex position in light space and store it into texture
    FragColor = vec4(length(v_LightSpacePos.xyz) + u_UserVariableFloat,
                    dot(v_LightSpacePos.xyz, v_LightSpacePos.xyz),
                    gl_FragCoord.z, // 0..1
                    0.0);
}
