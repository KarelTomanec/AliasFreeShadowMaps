#version 400 core

#ifdef USER_TEST
#endif

uniform int   u_UserVariableInt;
uniform float u_UserVariableFloat;

/*
// Built-in GLSL variables -----------
in gl_PerVertex {
    vec4  gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
    float gl_CullDistance[];
} gl_in[3];

out gl_PerVertex {
    vec4  gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
    float gl_CullDistance[];
};*/

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in Data {
    vec4 v_LightSpacePos;
    flat vec4 plane;
    flat vec3 triangle_vertices[3];
} In[];

out Data {
    smooth vec4 v_LightSpacePos;
    flat vec4 plane; // plane.xyz := n (plane normal), plane.w := d (dot(n,p) for a given point p on the plane)
    flat vec3 triangle_vertices[3];
} Out;

// Computes plane equation from three colinear points (ordered ccw)
vec4 computePlane(vec3 a, vec3 b, vec3 c);

void main() {
    
    vec4 triangle_plane = computePlane(In[0].v_LightSpacePos.xyz, In[1].v_LightSpacePos.xyz, In[2].v_LightSpacePos.xyz);

    for(int i = 0; i < gl_in.length(); i++) {
        for(int j = 0; j < gl_in.length(); j++) {
            Out.triangle_vertices[j] = In[j].v_LightSpacePos.xyz;
        }
        Out.plane = triangle_plane;
        Out.v_LightSpacePos = In[i].v_LightSpacePos;
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}

vec4 computePlane(vec3 a, vec3 b, vec3 c) {
    vec3 normal = normalize(cross(b - a, c - a));
    return vec4(normal, dot(normal, a));
}

