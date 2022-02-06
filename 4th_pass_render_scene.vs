#version 420 core

#ifdef USER_TEST
#endif

in vec4 a_Vertex;

void main() {

    gl_Position = vec4(a_Vertex.xy, 0.0f, 1.0f);
}
