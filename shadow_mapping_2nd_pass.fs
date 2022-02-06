#version 420 core

#ifdef USER_TEST
#endif

layout (location = 0) out vec4 FragColor;

in vec3 v_Normal;
in vec2 v_TexCoord;
in vec4 v_Vertex;
in vec4 v_LightSpacePos;

uniform int   u_UserVariableInt;
uniform float u_UserVariableFloat;
uniform vec4  u_LightPosition;
layout (binding = 0) uniform sampler2D       u_SceneTexture;
layout (binding = 1) uniform sampler2D       u_DepthMapTexture;
layout (binding = 2) uniform sampler2D       u_ZBufferTexture;
layout (binding = 3) uniform sampler2DShadow u_ZBufferShadowTexture;

void main() {
// Compute fragment diffuse color
    vec3 N      = normalize(v_Normal);
    vec3 L      = normalize(u_LightPosition.xyz - v_Vertex.xyz);
    float NdotL = max(dot(N, L), 0.0);
    vec4 color  = texture(u_SceneTexture, v_TexCoord) * NdotL;

    vec4 shadow = vec4(1.0);
    float depth = 0.0;
    float real_depth = 0.0;

    switch(u_UserVariableInt) 
    {
        case 0:
        {
            vec2 texCoord = -v_LightSpacePos.xy / v_LightSpacePos.z * 0.5 + 0.5; // * nearPlane 
            depth = texture(u_DepthMapTexture, texCoord).r;
            real_depth = length(v_LightSpacePos.xyz);
            shadow = (real_depth > depth) ? vec4(0.0) : vec4(1.0); // not effective -> use mix
            break;
        }
        case 1:
        {
            // texCoord ~ lightClipSpacePos
            vec3 texCoord = v_LightSpacePos.xyz / v_LightSpacePos.w * 0.5 + 0.5; 
            depth = texture(u_ZBufferTexture, texCoord.xy).r + u_UserVariableFloat * 0.001;
            real_depth = texCoord.z;
            shadow = (real_depth > depth) ? vec4(0.0) : vec4(1.0);
            break;
        }
        case 2:
        {
            vec3 texCoord = v_LightSpacePos.xyz / v_LightSpacePos.w * 0.5 + 0.5; 
            shadow = texture(u_ZBufferShadowTexture, texCoord.xyz).rrrr;
            break;
        }
        case 3:
        { 
            // FASTEST!
            shadow = textureProj(u_ZBufferShadowTexture, v_LightSpacePos).rrrr;
            break;
        }
    }
   

    // Modulate fragment's color according to result of shadow test
    FragColor = color* max(vec4(0.2), shadow);
}
