#version 430 core

#ifdef USER_TEST
#endif

layout (location = 0) out vec4 FragColor;

in vec4 v_Vertex;

uniform int   u_UserVariableInt;
uniform float u_UserVariableFloat;
uniform vec4  u_LightPosition;

layout (binding = 4, rgba8) uniform image2D lighting_map;

// Shadow map generated in the previous pass
layout (binding = 3, r32ui) uniform uimage2D shadow_map;

void main() {

    vec4 shadow = vec4(1.0);

    shadow = (imageLoad(shadow_map, ivec2(gl_FragCoord.xy)).x > 0) ? vec4(0.0) : vec4(1.0);
   
    // Modulate fragment's color according to result of shadow test
    FragColor = imageLoad(lighting_map, ivec2(gl_FragCoord.xy)) * max(vec4(0.2), shadow);
}
