//-----------------------------------------------------------------------------
//  [PGR2] Cube model
//  27/02/2008
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

#ifndef __CUBE_MODEL_H__
#define __CUBE_MODEL_H__

namespace Tools {
    namespace Mesh {
        const GLfloat CUBE_VERTEX_ARRAY[] = {
             -1.0f,-1.0f, 1.0f,  1.0f,-1.0f, 1.0f,  1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
             -1.0f,-1.0f,-1.0f, -1.0f, 1.0f,-1.0f,  1.0f, 1.0f,-1.0f,  1.0f,-1.0f,-1.0f,
             -1.0f, 1.0f,-1.0f, -1.0f, 1.0f, 1.0f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,-1.0f,
             -1.0f,-1.0f,-1.0f,  1.0f,-1.0f,-1.0f,  1.0f,-1.0f, 1.0f, -1.0f,-1.0f, 1.0f,
              1.0f,-1.0f,-1.0f,  1.0f, 1.0f,-1.0f,  1.0f, 1.0f, 1.0f,  1.0f,-1.0f, 1.0f,
             -1.0f,-1.0f,-1.0f, -1.0f,-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f,-1.0f
        };

        const GLfloat CUBE_VERTEX_ARRAY_TRIANGLE_VERTICES_4D[] = {
           -1.0f,-1.0f, 1.0f, 1.0f,    1.0f,-1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,
           -1.0f,-1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,   -1.0f, 1.0f, 1.0f, 1.0f,
           -1.0f,-1.0f,-1.0f, 1.0f,   -1.0f, 1.0f,-1.0f, 1.0f,    1.0f, 1.0f,-1.0f, 1.0f,
           -1.0f,-1.0f,-1.0f, 1.0f,    1.0f, 1.0f,-1.0f, 1.0f,    1.0f,-1.0f,-1.0f, 1.0f,
           -1.0f, 1.0f,-1.0f, 1.0f,   -1.0f, 1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,
           -1.0f, 1.0f,-1.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    1.0f, 1.0f,-1.0f, 1.0f,
           -1.0f,-1.0f,-1.0f, 1.0f,    1.0f,-1.0f,-1.0f, 1.0f,    1.0f,-1.0f, 1.0f, 1.0f,
           -1.0f,-1.0f,-1.0f, 1.0f,    1.0f,-1.0f, 1.0f, 1.0f,   -1.0f,-1.0f, 1.0f, 1.0f,
            1.0f,-1.0f,-1.0f, 1.0f,    1.0f, 1.0f,-1.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,
            1.0f,-1.0f,-1.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    1.0f,-1.0f, 1.0f, 1.0f,
           -1.0f,-1.0f,-1.0f, 1.0f,   -1.0f,-1.0f, 1.0f, 1.0f,   -1.0f, 1.0f, 1.0f, 1.0f,
           -1.0f,-1.0f,-1.0f, 1.0f,   -1.0f, 1.0f, 1.0f, 1.0f,   -1.0f, 1.0f,-1.0f, 1.0f
        };

        const GLfloat CUBE_MAP_VERTEX_ARRAY[] = {
            // Vertex coords      Normal coords    
             1.0f,-1.0f,-1.0f,    1.0f, 0.0f, 0.0f,
             1.0f, 1.0f,-1.0f,    1.0f, 0.0f, 0.0f,
             1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 0.0f,
             1.0f,-1.0f,-1.0f,    1.0f, 0.0f, 0.0f,
             1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 0.0f,
             1.0f,-1.0f, 1.0f,    1.0f, 0.0f, 0.0f,

            -1.0f,-1.0f,-1.0f,   -1.0f, 0.0f, 0.0f,
            -1.0f,-1.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
            -1.0f, 1.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
            -1.0f,-1.0f,-1.0f,   -1.0f, 0.0f, 0.0f,
            -1.0f, 1.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
            -1.0f, 1.0f,-1.0f,   -1.0f, 0.0f, 0.0f,

            -1.0f, 1.0f,-1.0f,    0.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 0.0f,
             1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 0.0f,
            -1.0f, 1.0f,-1.0f,    0.0f, 1.0f, 0.0f,
             1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 0.0f,
             1.0f, 1.0f,-1.0f,    0.0f, 1.0f, 0.0f,

            -1.0f,-1.0f,-1.0f,    0.0f,-1.0f, 0.0f,
             1.0f,-1.0f,-1.0f,    0.0f,-1.0f, 0.0f,
             1.0f,-1.0f, 1.0f,    0.0f,-1.0f, 0.0f, 
            -1.0f,-1.0f,-1.0f,    0.0f,-1.0f, 0.0f,
             1.0f,-1.0f, 1.0f,    0.0f,-1.0f, 0.0f, 
            -1.0f,-1.0f, 1.0f,    0.0f,-1.0f, 0.0f,

            -1.0f,-1.0f, 1.0f,    0.0f, 0.0f, 1.0f,
             1.0f,-1.0f, 1.0f,    0.0f, 0.0f, 1.0f,
             1.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f,
            -1.0f,-1.0f, 1.0f,    0.0f, 0.0f, 1.0f,
             1.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f,

            -1.0f,-1.0f,-1.0f,    0.0f, 0.0f,-1.0f,
            -1.0f, 1.0f,-1.0f,    0.0f, 0.0f,-1.0f,
             1.0f, 1.0f,-1.0f,    0.0f, 0.0f,-1.0f,
            -1.0f,-1.0f,-1.0f,    0.0f, 0.0f,-1.0f,
             1.0f, 1.0f,-1.0f,    0.0f, 0.0f,-1.0f,
             1.0f,-1.0f,-1.0f,    0.0f, 0.0f,-1.0f,
        };

        const GLfloat CUBE_COLOR_ARRAY[] = {
            1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f, 0.0f,
            1.0f, 0.0f, 1.0f,  0.0f, 1.0f, 1.0f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 1.0f,  1.0f, 1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f, 1.0f,
            1.0f, 0.0f, 1.0f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f,
            1.0f, 0.0f, 1.0f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f, 1.0f,
        };

        const GLfloat CUBE_TEX_COORD_ARRAY[] = {
             0.0f, 0.0f,  1.0f, 0.0f,  1.0f, 1.0f,  0.0f, 1.0f,
             0.0f, 0.0f,  1.0f, 0.0f,  1.0f, 1.0f,  0.0f, 1.0f,
             0.0f, 0.0f,  1.0f, 0.0f,  1.0f, 1.0f,  0.0f, 1.0f,
             0.0f, 0.0f,  1.0f, 0.0f,  1.0f, 1.0f,  0.0f, 1.0f,
             0.0f, 0.0f,  1.0f, 0.0f,  1.0f, 1.0f,  0.0f, 1.0f,
             0.0f, 0.0f,  1.0f, 0.0f,  1.0f, 1.0f,  0.0f, 1.0f,
        };

        const GLfloat CUBE_NORMAL_ARRAY[] = {
            0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
            0.0f, 0.0f,-1.0f,  0.0f, 0.0f,-1.0f,  0.0f, 0.0f,-1.0f,  0.0f, 0.0f,-1.0f,
            0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
            0.0f,-1.0f, 0.0f,  0.0f,-1.0f, 0.0f,  0.0f,-1.0f, 0.0f,  0.0f,-1.0f, 0.0f,
            1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
           -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        };

        const GLfloat CUBE_COLOR_AND_VERTEX_ARRAY[] = {
           // <cr, cg, cb>, <vx, vy, vx>
            1.0f, 0.0f, 0.0f, -1.0f,-1.0f, 1.0f,   0.0f, 1.0f, 0.0f,  1.0f,-1.0f, 1.0f,
            0.0f, 0.0f, 1.0f,  1.0f, 1.0f, 1.0f,   1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 1.0f,
            1.0f, 0.0f, 1.0f, -1.0f,-1.0f,-1.0f,   0.0f, 1.0f, 1.0f, -1.0f, 1.0f,-1.0f,
            1.0f, 1.0f, 1.0f,  1.0f, 1.0f,-1.0f,   1.0f, 0.0f, 0.0f,  1.0f,-1.0f,-1.0f,  
            0.0f, 1.0f, 1.0f, -1.0f, 1.0f,-1.0f,   1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 1.0f,
            0.0f, 0.0f, 1.0f,  1.0f, 1.0f, 1.0f,   1.0f, 1.0f, 1.0f,  1.0f, 1.0f,-1.0f,
            1.0f, 0.0f, 1.0f, -1.0f,-1.0f,-1.0f,   1.0f, 0.0f, 0.0f,  1.0f,-1.0f,-1.0f,
            0.0f, 1.0f, 0.0f,  1.0f,-1.0f, 1.0f,   1.0f, 0.0f, 0.0f, -1.0f,-1.0f, 1.0f,
            1.0f, 0.0f, 0.0f,  1.0f,-1.0f,-1.0f,   1.0f, 1.0f, 1.0f,  1.0f, 1.0f,-1.0f,
            0.0f, 0.0f, 1.0f,  1.0f, 1.0f, 1.0f,   0.0f, 1.0f, 0.0f,  1.0f,-1.0f, 1.0f,
            1.0f, 0.0f, 1.0f, -1.0f,-1.0f,-1.0f,   1.0f, 0.0f, 0.0f, -1.0f,-1.0f, 1.0f,
            1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 1.0f,   0.0f, 1.0f, 1.0f, -1.0f, 1.0f,-1.0f,
        };

        const GLubyte CUBE_INDEX_ARRAY[] = {
           0, 1, 2, 3,
            4, 5, 6, 7,
            5, 3, 2, 6,
            4, 7, 1, 0,
            7, 6, 2, 1,
            4, 0, 3, 5
        };

        const int NUM_CUBE_VERTICES = sizeof(CUBE_VERTEX_ARRAY) / (3*sizeof(GLfloat));
        const int NUM_CUBE_INDICES  = sizeof(CUBE_INDEX_ARRAY) / sizeof(GLubyte);
    } // end of namespace Mesh

    //-----------------------------------------------------------------------------
    // Name: DrawCube()
    // Desc: 
    //-----------------------------------------------------------------------------
    inline void DrawCube() {
        static GLuint s_vao = 0;
        if (s_vao == 0) {
            // Lazy initialization of VBO with cube polygon mesh
            glGenVertexArrays(1, &s_vao);
            glBindVertexArray(s_vao);
                GLuint vbo = 0;
                glGenBuffers(1, &vbo);
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBufferData(GL_ARRAY_BUFFER, sizeof(Mesh::CUBE_VERTEX_ARRAY), Mesh::CUBE_VERTEX_ARRAY, GL_STATIC_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (const void*) 0);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (const void*) 0);
                glEnableVertexAttribArray(0);
                glEnableVertexAttribArray(1);
            glBindVertexArray(0);
            glDeleteBuffers(1, &vbo);
        }

        glBindVertexArray(s_vao);
        glDrawArrays(GL_QUADS, 0, sizeof(Mesh::CUBE_VERTEX_ARRAY) / (3*sizeof(GLfloat)));
        glBindVertexArray(0);
    }
} // end of namespace Tools

#endif // __CUBE_MODEL_H__