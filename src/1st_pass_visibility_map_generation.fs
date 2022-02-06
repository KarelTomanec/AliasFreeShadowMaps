#version 430 core

layout (location = 0) out vec4 FragColor0;
layout (location = 1) out vec4 FragColor1;

layout (location = 3) uniform vec4  u_LightPosition;

layout (binding = 0) uniform sampler2D u_SceneTexture;

in vec4 v_LightSpacePos;
in vec3 v_Normal;
in vec2 v_TexCoord;
in vec4 v_Vertex;

void main(void) {

    // Compute fragment diffuse color
    vec3 N      = normalize(v_Normal);
    vec3 L      = normalize(u_LightPosition.xyz - v_Vertex.xyz);
    float NdotL = max(dot(N, L), 0.0);
    vec4 color  = texture(u_SceneTexture, v_TexCoord) * NdotL;

    FragColor0 = vec4(v_LightSpacePos.xyz, 1.0);
    FragColor1 = color;

}
