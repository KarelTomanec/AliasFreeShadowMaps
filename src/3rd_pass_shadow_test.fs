#version 430 core

#ifdef USER_TEST
#endif

uniform int   u_UserVariableInt;
uniform float u_UserVariableFloat;

// Camera visibility
layout (binding = 1) uniform sampler2D visibility_map;

// Head pointer 2D buffer
layout (binding = 2) uniform usampler2D head_pointer_image;

// Linked list 1D buffer
layout (binding = 2, rgba32ui) uniform uimageBuffer list_buffer;

// Shadow map 2D texture
layout (binding = 3, r32ui) uniform uimage2D shadow_map;

in Data {
    smooth vec4 v_LightSpacePos;
    flat vec4 plane; // plane.xyz := n (plane normal), plane.w := d (dot(n,p) for a given point p on the plane)
    flat vec3 triangle_vertices[3];
} In;

const float SHADOW_ACNE_EPSILON = 0.0001;


// Test if point p lies inside the ccw triangle
bool pointInsideTriangle(vec3 p);

// Test if shadow ray starting from the point a in the direction v intersects triangle
bool intersectRayTriangle(vec3 a, vec3 v);

void main(void) {

    uint curr_index = texelFetch(head_pointer_image, ivec2(gl_FragCoord.xy), 0).x;

    // Linked list traversal
    while(curr_index != 0xFFFFFFFF) {

        // Read an entry from the linked list
        uvec4 entry = imageLoad(list_buffer, int(curr_index));

        // entry.x contains pointer to the next entry in the linked list
        curr_index = entry.x;

        // entry.yz contains coordinates of point in the visibility map
        ivec2 visibility_map_coord = ivec2(entry.yz);

        // Transform point in world coordinates to the light space
        vec4 p = texelFetch(visibility_map, visibility_map_coord, 0);

        // Test if the fragment lies in shadow
        if(intersectRayTriangle(p.xyz + normalize(-p.xyz) * SHADOW_ACNE_EPSILON, -p.xyz)) {
            imageStore(shadow_map, ivec2(visibility_map_coord), uvec4(1));
        } 
    }
}

bool pointInsideTriangle(vec3 p) {

    // Translate point and triangle so that point lies at origin
    vec3 a = In.triangle_vertices[0] - p; 
    vec3 b = In.triangle_vertices[1] - p; 
    vec3 c = In.triangle_vertices[2] - p;
    float ab = dot(a, b);
    float ac = dot(a, c);
    float bc = dot(b, c);
    float cc = dot(c, c);

    // Make sure plane normals for triangles pab and pbc point in the same direction
    if (bc * ac - cc * ab < 0.0) return false;

    // Make sure plane normals for triangles pab and pca point in the same direction
    float bb = dot(b, b);
    if (ab * bc - ac * bb < 0.0) return false;

    // Otherwise p must be in or on the triangle
    return true;
}

bool intersectRayTriangle(vec3 a, vec3 v) {
    
    // Intersect plane
    float t = (In.plane.w - dot(In.plane.xyz, a)) / dot(In.plane.xyz, v);

    if(0.0 <= t) {
        // Plane was intersected, check if the intersection lies inside the triangle
        return pointInsideTriangle(a + t * v);
    }

    return false;
}
