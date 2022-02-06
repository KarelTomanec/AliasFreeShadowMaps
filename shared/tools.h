//-----------------------------------------------------------------------------
//  [PGR2] Some helpful common functions
//  19/02/2014
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
#include <math.h>
#include <string>
#include <vector>
#include "./glm/gtx/integer.hpp"
#include "./glm/core/func_exponential.hpp"
#include "models/scene_miro.h"

// INTERNAL VARIABLES DEFINITIONS______________________________________________
namespace Variables {
    struct Transformation { // Scene transformation matrixes ( readonly variables - calculated automatically every frame)
        glm::mat4 ModelView;
        glm::mat4 ModelViewInverse;
        glm::mat4 Projection;
        glm::mat4 ModelViewProjection;
        glm::vec4 Viewport;
        glm::mat4 Model;
    };

    GLFWwindow*    Window            = nullptr;
    glm::ivec2     WindowSize        = glm::ivec2(0);
    glm::ivec2     LastWindowSize    = glm::ivec2(0);
    glm::ivec2     MousePosition     = glm::ivec2(-1, -1);
    bool           MouseLeftPressed  = false;
    bool           MouseRightPressed = false;
    bool           Debug             = false;
    bool           ShowMemStat       = true;
    bool           AppClose          = false;
    Transformation Transform;


    namespace Zoom {
        glm::ivec2 Center      = glm::ivec2(0);
        bool       Enabled     = false;
        int        Factor      = 2;
    };

    namespace Shader {              // Shader default variables
        int   Int       =    0;     // Value will be automatically passed to 'u_UserVariableInt' uniform in all shaders
        float Float     = 0.0f;     // Value will be automatically passed to 'u_UserFloatInt' uniform in all shaders

        glm::vec4 SceneRotation = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // Scene orientation
        GLfloat   SceneZOffset  = 2.0f;                              // Scene translation along z-axis
        bool      UserTest      = false;                             // USER_TEST will be define in every shader if true
    }; // end of namespace Variables::Shader

    namespace Menu {
        int   Int0      =    0;     // Value will be automatically shown in the sub-menu 'Statistic'
        float Float0    = 0.0f;     // Value will be automatically shown in the sub-menu 'Statistic'
        int   Int1      =    0;     // Value will be automatically shown in the sub-menu 'Statistic'
        float Float1    = 0.0f;     // Value will be automatically shown in the sub-menu 'Statistic'
    }; // end of namespace Variables::Menu
}; // end of namespace Variables

namespace Statistic {
    float FPS = 0.0f;               // Current FPS

    namespace Frame {
        int   GPUTime   = 0;        // GPU frame time
        int   CPUTime   = 0;        // CPU frame time
        int   ID        = 0;        // Id of the frame
    }; // end of namespace Statistic::Frame

    namespace GPUMemory {
        int DedicatedMemory = 0;    // Size of dedicated GPU memory
        int TotalMemory     = 0;    // Size of total GPU memory
        int AllocatedMemory = 0;    // Size of allocated GPU memory
        int FreeMemory      = 0;    // Size of available GPU memory
        int EvictedMemory   = 0;    // Size of available GPU memory
    }; // end of namespace Statistic::GPUMemory
}; // end of namespace Statistic

namespace OpenGL {
    enum eQueries {
        FrameStartQuery = 0, FrameEndQuery, NumQueries
    };

    struct Program {
        Program(GLuint _id) : id(_id) {
            MVPMatrix          = glGetUniformLocation(id, "u_MVPMatrix");
            ModelViewMatrix    = glGetUniformLocation(id, "u_ModelViewMatrix");
            ModelViewMatrixInv = glGetUniformLocation(id, "u_ModelViewMatrixInverse");
            ProjectionMatrix   = glGetUniformLocation(id, "u_ProjectionMatrix");
            Viewport           = glGetUniformLocation(id, "u_Viewport");
            ZOffset            = glGetUniformLocation(id, "u_ZOffset");
            UserVariableInt    = glGetUniformLocation(id, "u_UserVariableInt");
            UserVariableFloat  = glGetUniformLocation(id, "u_UserVariableFloat");
            FrameCounter       = glGetUniformLocation(id, "u_FrameCounter");
        }
        bool hasMVMatrix() const {
            return (MVPMatrix > -1) || (ModelViewMatrix > -1) || (ModelViewMatrixInv > -1);
        }
        bool hasZOffset() const {
            return (ZOffset > -1);
        }
        GLuint id;
        GLint  MVPMatrix;
        GLint  ModelViewMatrix;
        GLint  ModelViewMatrixInv;
        GLint  ProjectionMatrix;
        GLint  Viewport;
        GLint  ZOffset;
        GLint  UserVariableInt;
        GLint  UserVariableFloat;
        GLint  FrameCounter;
    };
    std::vector<Program> programs;

    GLuint Query[NumQueries] = {0};
}; // end of namespace OpenGL


namespace Tools {
    //-----------------------------------------------------------------------------
    // Name: GetCPUTime()
    // Desc: 
    //-----------------------------------------------------------------------------
    inline unsigned int GetCPUTime() {
        return static_cast<unsigned int>(glfwGetTime() * 1000000);
    }


    //-----------------------------------------------------------------------------
    // Name: GetGPUTime()
    // Desc: 
    //-----------------------------------------------------------------------------
    inline GLint64 GetGPUTime() {
        GLint64 time = 0;
        glGetInteger64v(GL_TIMESTAMP, &time);
        return time;
    }


    // Very simple GPU timer (timer cannot be nested with other timer or GL_TIME_ELAPSED query
    class GPUTimer {
    public:
        GPUTimer() : query(0), time(0), time_total(0), counter(0) {}
        ~GPUTimer() {
            glDeleteQueries(1, &query);
        }

        void start() {
            if (query == 0) glCreateQueries(GL_TIME_ELAPSED, 1, &query);
            glBeginQuery(GL_TIME_ELAPSED, query);
        }

        void stop() const {
            glEndQuery(GL_TIME_ELAPSED);
        }

        unsigned int get() {
            glGetQueryObjectui64v(query, GL_QUERY_RESULT, &time);
            time_total += time;
            counter++;
            return static_cast<unsigned int>(time);
        }

        unsigned int getAverage() const {
            return static_cast<unsigned int>(static_cast<double>(time_total) / counter + 0.5);
        }

        unsigned int getCounter() const {
            return counter;
        }

        void clear() {
            time_total = 0;
            counter    = 0;
        }

    private:
        GLuint   query;
        GLuint64 time;
        GLuint64 time_total;
        GLuint   counter;
    };

    // Very simple CPU timer
    class CPUTimer {
    public:
        CPUTimer() : start_time(0), time(0), time_total(0), counter(0) {}

        void start() {
            start_time = Tools::GetCPUTime();
        }

        void stop() {
            time = Tools::GetCPUTime() - start_time;
        }

        unsigned int get() {
            time_total += time;
            counter++;
            return static_cast<unsigned int>(time);
        }

        unsigned int getAverage() const {
            return static_cast<unsigned int>(static_cast<double>(time_total) / counter + 0.5);
        }

        unsigned int getCounter() const {
            return counter;
        }

        void clear() {
            time_total = 0;
            counter    = 0;
        }

    private:
        unsigned int       start_time;
        unsigned int       time;
        unsigned long long time_total;
        unsigned int       counter;
    };

    //-----------------------------------------------------------------------------
    // Name: SaveFrambuffer()
    // Desc: 
    //-----------------------------------------------------------------------------
    void SaveFrambuffer(GLuint framebufferId, GLsizei width, GLsizei height) {
        GLsizei const numPixels = width * height;
        std::vector<unsigned char> pixels(numPixels * 4);

        GLint fboId = 0;
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &fboId);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferId);
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);
        char file_name[50] = { 0 };
        sprintf(file_name, "framebuffer_%dx%d.raw", width, height);
        FILE * file = fopen(file_name, "wb");
        if (file) {
            fwrite(&pixels[0], numPixels, 4 * sizeof(unsigned char), file);
            fclose(file);
        }
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fboId);
    }


    //-----------------------------------------------------------------------------
    // Name: OpenFile()
    // Desc: 
    //-----------------------------------------------------------------------------
    FILE* OpenFile(const char* file_name) {
        if (!file_name)
            return nullptr;

        FILE* fin = fopen(file_name, "rb");

#ifdef PROJECT_DIRECTORY
        if (!fin) {
            std::string absolute_path = std::string(PROJECT_DIRECTORY) + file_name;
            fin = fopen(absolute_path.c_str(), "rb");
        }
#endif
        return fin;

    }

    //-----------------------------------------------------------------------------
    // Name: ReadFile()
    // Desc: 
    //-----------------------------------------------------------------------------
    char* ReadFile(const char* file_name, size_t* bytes_read = 0) {
        char* buffer = nullptr;

        // Read input file
        FILE* fin = OpenFile(file_name);
        if (fin) {
            fseek(fin, 0, SEEK_END);
            long file_size = ftell(fin);
            rewind(fin);

            if (file_size > 0) {
                buffer = new char[file_size + 1];
                size_t count = fread(buffer, sizeof(char), file_size, fin);
                buffer[count] = '\0';
                if (bytes_read) *bytes_read = count;
            }
            fclose(fin);
        }

        return buffer;
    }


    namespace Mesh {
        //-----------------------------------------------------------------------------
        // Name: CreatePlane()
        // Desc: 
        //-----------------------------------------------------------------------------
        glm::vec3* CreatePlane(float size, int slice_x, int slice_y, int* num_vertices) {
            assert(num_vertices);

            const int NUM_VERTICES = 4*slice_x*slice_y;
            if (NUM_VERTICES < 4)
                return nullptr;

            // Compute position deltas for moving down the X, and Z axis during mesh creation
            const glm::vec3 delta = glm::vec3(size / slice_x, size / slice_y, 0.0f);
            const glm::vec3 start = glm::vec3(-size * 0.5f, -size * 0.5f, 0.0f);   
            glm::vec3* mesh = new glm::vec3[NUM_VERTICES];
            glm::vec3* ptr  = mesh;

            for (int x = 0; x < slice_x; x++) {
                for (int y = 0; y < slice_y; y++) {
                    *ptr++ = start + delta * glm::vec3(    x,     y, 0.0f);
                    *ptr++ = start + delta * glm::vec3(x + 1,     y, 0.0f);
                    *ptr++ = start + delta * glm::vec3(x + 1, y + 1, 0.0f);
                    *ptr++ = start + delta * glm::vec3(    x, y + 1, 0.0f);
                }
            }

            if (!num_vertices)
                *num_vertices = NUM_VERTICES;

            return mesh;
        }


        //-----------------------------------------------------------------------------
        // Name: CreatePlane()
        // Desc: 
        //-----------------------------------------------------------------------------
        bool CreatePlane(GLsizei vertex_density, GLsizei index_density, GLenum mode, 
                         std::vector<glm::vec2>& vertices, std::vector<GLuint>& indices) {
            if ((vertex_density < 2) || ((index_density < 2) && (index_density != 0)))
                return false;
            if ((mode != GL_TRIANGLES) && (mode != GL_TRIANGLE_STRIP) && (mode != GL_QUADS))
                return false;

        // Generate mesh vertices
            const int total_vertices = vertex_density*vertex_density;
            const GLfloat offset = 2.0f / (vertex_density-1);

            vertices.clear();
            vertices.reserve(total_vertices);
            for (int y = 0; y < vertex_density - 1; y++) {
                for (int x = 0; x < vertex_density - 1; x++)
                    vertices.push_back(glm::vec2(-1.0f + x*offset, -1.0f + y*offset));
                vertices.push_back(glm::vec2(1.0f, -1.0f + y*offset));
            }
            for (int x = 0; x < vertex_density - 1; x++)
                vertices.push_back(glm::vec2(-1.0f + x*offset, 1.0f));
            vertices.push_back(glm::vec2(1.0f, 1.0f));

        // Generate mesh indices
            indices.clear();
            if (index_density == 0) {
                const int total_indices = (mode == GL_TRIANGLES) ? 6*(vertex_density - 1)*(vertex_density - 1) : 2*(vertex_density*vertex_density - 2);
                indices.reserve(total_indices);

                GLuint index  = 0;
                if (mode == GL_TRIANGLES) {
                    for (int y = 0; y < vertex_density - 1; y++) {
                        for (int x = 0; x < vertex_density - 1; x++) {
                            indices.push_back(index);
                            indices.push_back(index + 1);
                            indices.push_back(index + vertex_density);
                            indices.push_back(index + vertex_density);
                            indices.push_back(index + 1);
                            indices.push_back(index + 1 + vertex_density);
                            index++;
                        }
                        index++;
                    }
                }
                else {
                    for (int y = 0; y < vertex_density - 1; y++) {
                        for (int x = 0; x < vertex_density; x++) {
                            indices.push_back(index);
                            indices.push_back(index + vertex_density);
                            index++;
                        }
                        if ((vertex_density > 2) && (y != vertex_density - 2)) {
                            indices.push_back(index + vertex_density - 1);
                            indices.push_back(index);
                        }
                    }
                }
            } else { // (index_density == 0)
                const int total_indices = (mode == GL_TRIANGLES) ? 6*(index_density - 1)*(index_density - 1) : 
                                          ((mode == GL_TRIANGLE_STRIP) ? 2*(index_density*index_density - 2) : 4*(index_density - 1)*(index_density - 1));
                const float h_index_offset = static_cast<float>(vertex_density - 1) / (index_density - 1);
                indices.reserve(total_indices);

                GLuint index  = 0;
                switch (mode) {
                case GL_TRIANGLES: {
                        for (int y = 0; y < index_density - 1; y++) {
                            const GLuint vindex1 = glm::clamp(static_cast<int>(glm::round(y * h_index_offset)*vertex_density), 0, vertex_density*(vertex_density - 1));
                            const GLuint vindex2 = glm::clamp(static_cast<int>(glm::round((y + 1)*h_index_offset) * vertex_density), 0, vertex_density*(vertex_density - 1));
                            for (int x = 0; x < index_density - 1; x++) {
                                const GLuint hindex1 = glm::clamp(static_cast<int>(x * h_index_offset + 0.5f), 0, vertex_density - 1);
                                const GLuint hindex2 = glm::clamp(static_cast<int>((x + 1) * h_index_offset + 0.5f), 0, vertex_density - 1);
                                indices.push_back(vindex1 + hindex1); // 0
                                indices.push_back(vindex1 + hindex2); // 1
                                indices.push_back(vindex2 + hindex1); // 2
                                indices.push_back(vindex2 + hindex1); // 3
                                indices.push_back(vindex1 + hindex2); // 4
                                indices.push_back(vindex2 + hindex2); // 5
                            }
                        }
                    }
                    break;
                case GL_TRIANGLE_STRIP: {
                        for (int y = 0; y < index_density - 1; y++) {
                            const GLuint vindex1 = glm::clamp(static_cast<int>(glm::round(y * h_index_offset)*vertex_density), 0, vertex_density*(vertex_density - 1));
                            const GLuint vindex2 = glm::clamp(static_cast<int>(glm::round((y + 1)*h_index_offset) * vertex_density), 0, vertex_density*(vertex_density - 1));
                            const int startIdx = indices.size() + 1;
                            const int endIdx = indices.size() + 2*index_density - 1;
                            for (int x = 0; x < index_density; x++) {
                                const GLuint hindex = glm::clamp(static_cast<int>(x * h_index_offset + 0.5f), 0, vertex_density - 1);
                                indices.push_back(hindex + vindex1);
                                indices.push_back(hindex + vindex2);
                            }
                            if (y < index_density - 2) {
                                // add 2 degenerated triangles
                                indices.push_back(indices[endIdx]);
                                indices.push_back(indices[startIdx]);
                            }
                        }
                    }
                    break;
                case GL_QUADS: {
                        for (int y = 0; y < index_density - 1; y++) {
                            const GLuint vindex1 = glm::clamp(static_cast<int>(glm::round(y * h_index_offset)*vertex_density), 0, vertex_density*(vertex_density - 1));
                            const GLuint vindex2 = glm::clamp(static_cast<int>(glm::round((y + 1)*h_index_offset) * vertex_density), 0, vertex_density*(vertex_density - 1));
                            for (int x = 0; x < index_density - 1; x++) {
                                const GLuint hindex1 = glm::clamp(static_cast<int>(x * h_index_offset + 0.5f), 0, vertex_density - 1);
                                const GLuint hindex2 = glm::clamp(static_cast<int>((x + 1) * h_index_offset + 0.5f), 0, vertex_density - 1);
                                indices.push_back(hindex1 + vindex1);
                                indices.push_back(hindex1 + vindex2);
                                indices.push_back(hindex2 + vindex2);
                                indices.push_back(hindex2 + vindex1);
                            }
                        }
                    }
                    break;
                default:
                    assert(0);
                    break;
                }
            }
    
            return true;
        }

        //-----------------------------------------------------------------------------
        // Name: CreateCircle()
        // Desc: 
        //-----------------------------------------------------------------------------
        template <class INDEX_TYPE>
        void CreateCircle(GLenum primitive_type, unsigned int num_triangles, float z,
                          std::vector<glm::vec3>& vertices, std::vector<INDEX_TYPE>* indices) {
            if (((primitive_type != GL_TRIANGLES) && (primitive_type != GL_TRIANGLE_FAN)) || (num_triangles < 4))
                return;

            const GLfloat TWO_PI = 2.0f * glm::pi<float>();
            const GLfloat alpha_step = TWO_PI / num_triangles;
            GLfloat alpha = 0.0f;

            if (indices) {
                // Generate mesh vertices
                vertices.reserve(num_triangles + 1);
                vertices.push_back(glm::vec3());
                for (unsigned int iTriangle = 0; iTriangle < num_triangles; iTriangle++) {
                    vertices.push_back(glm::vec3(cos(alpha), sin(alpha), z));
                    alpha = glm::min(TWO_PI, alpha + alpha_step);
                }

                // Generate mesh indices
                if (primitive_type == GL_TRIANGLES) {
                    indices->reserve(num_triangles * 3);
                    for (unsigned int iTriangle = 0; iTriangle < num_triangles - 1; iTriangle++) {
                        indices->push_back(static_cast<INDEX_TYPE>(0));
                        indices->push_back(static_cast<INDEX_TYPE>(iTriangle + 1));
                        indices->push_back(static_cast<INDEX_TYPE>(iTriangle + 2));
                    }
                    indices->push_back(static_cast<INDEX_TYPE>(0));
                    indices->push_back(static_cast<INDEX_TYPE>(num_triangles));
                    indices->push_back(static_cast<INDEX_TYPE>(1));
                } else {
                    indices->reserve(vertices.size() + 1);
                    for (unsigned int iVertex = 0; iVertex < vertices.size(); iVertex++) {
                        indices->push_back(static_cast<INDEX_TYPE>(iVertex));
                    }
                    indices->push_back(static_cast<INDEX_TYPE>(1));
                }
            }
            else {
            // Generate sequence geometry mesh
                vertices.reserve((primitive_type == GL_TRIANGLES) ? (num_triangles * 3) : (num_triangles + 2));
                for (unsigned int iTriangle = 0; iTriangle < num_triangles; iTriangle++) {
                    // 1st vertex of triangle
                    if ((primitive_type == GL_TRIANGLES) || (iTriangle == 0))
                        vertices.push_back(glm::vec3());

                    // 2nd vertex of triangle
                    vertices.push_back(glm::vec3(cos(alpha), sin(alpha), z));
                    // 3rd vertex of triangle
                    alpha = glm::min(TWO_PI, alpha + alpha_step);
                    if (iTriangle == num_triangles - 1)
                        vertices.push_back(glm::vec3(1.0f, 0.0f, z));
                    else if (primitive_type == GL_TRIANGLES)
                        vertices.push_back(glm::vec3(cos(alpha), sin(alpha), z));
                }
            }
        }


        //-----------------------------------------------------------------------------
        // Name: CreateSphere()
        // Desc: based on OpenGL SuperBible example by Richard S. Wright Jr.
        //-----------------------------------------------------------------------------
        bool CreateSphereMesh(float radius, int slices, int stacks, std::vector<glm::vec3> & vertices) {
            const int NUM_VERTICES = stacks*(2 * slices + 2);
            if (NUM_VERTICES < 1)
                return false;

            const float DRHO = glm::pi<float>() / stacks;
            const float DTHETA = 2.0f * glm::pi<float>() / slices;
            vertices.reserve(vertices.size() + NUM_VERTICES);
            for (int iStack = 0; iStack < stacks; iStack++) {
                const GLfloat RHO = iStack * DRHO;
                for (int iSlice = 0; iSlice <= slices; iSlice++) {
                    const GLfloat THETA   = (iSlice == slices) ? 0.0f : iSlice * DTHETA;
                    const glm::vec3 theta = glm::vec3((GLfloat)(-sin(THETA)), (GLfloat)(cos(THETA)), 1.0f) * radius;
                    vertices.emplace_back(theta * glm::vec3((GLfloat)(sin(RHO + DRHO)), (GLfloat)(sin(RHO + DRHO)), (GLfloat)(cos(RHO + DRHO))));
                    vertices.emplace_back(theta * glm::vec3((GLfloat)(sin(RHO)), (GLfloat)(sin(RHO)), (GLfloat)(cos(RHO))));
                }
            }

            return true;
        }
    } // end of namespace Mesh



    namespace Shader {
        //-----------------------------------------------------------------------------
        // Name: CheckShaderInfoLog()
        // Desc: 
        //-----------------------------------------------------------------------------
        void CheckShaderInfoLog(GLuint shader_id) {
            if (shader_id == 0)
                return;
    
            int log_length = 0;
            glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);

            if (log_length > 0) {
                char* buffer  = new char[log_length];
                int   written = 0;
                glGetShaderInfoLog(shader_id, log_length, &written, buffer);
                fprintf(stderr, "%s\n", buffer);
                delete [] buffer;
            }
        }

        //-----------------------------------------------------------------------------
        // Name: CheckProgramInfoLog()
        // Desc: 
        //-----------------------------------------------------------------------------
        void CheckProgramInfoLog(GLuint program_id) {
            if (program_id == 0)
                return;
 
            int log_length = 0;
            glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);

            if (log_length > 0) {
                char* buffer  = new char[log_length];
                int   written = 0;
                glGetProgramInfoLog(program_id, log_length, &written, buffer);
                fprintf(stderr, "%s\n", buffer);
                delete [] buffer;
            }
        }

        //-----------------------------------------------------------------------------
        // Name: CheckShaderCompileStatus()
        // Desc: 
        //-----------------------------------------------------------------------------
        GLint CheckShaderCompileStatus(GLuint shader_id) {
            GLint status = GL_FALSE;
            glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);
            return status;
        }


        //-----------------------------------------------------------------------------
        // Name: CheckProgramLinkStatus()
        // Desc: 
        //-----------------------------------------------------------------------------
        GLint CheckProgramLinkStatus(GLuint program_id) {
            GLint status = GL_FALSE;
            glGetProgramiv(program_id, GL_LINK_STATUS, &status);
            return status;
        }


        //-----------------------------------------------------------------------------
        // Name: CreateShaderFromSource()
        // Desc: 
        //-----------------------------------------------------------------------------
        GLuint CreateShaderFromSource(GLenum shader_type, const char* source) {
            if (!source)
                return 0;

            switch (shader_type) {
                case GL_VERTEX_SHADER         : fprintf(stderr, "vertex shader creation ... "); break;
                case GL_FRAGMENT_SHADER       : fprintf(stderr, "fragment shader creation ... "); break;
                case GL_GEOMETRY_SHADER       : fprintf(stderr, "geometry shader creation ... "); break;
                case GL_TESS_CONTROL_SHADER   : fprintf(stderr, "tesselation control shader creation ... "); break;
                case GL_TESS_EVALUATION_SHADER: fprintf(stderr, "tesselation evaluation shader creation ... "); break;
                default                       : return 0;
            }

            GLuint shader_id = glCreateShader(shader_type);
            if (shader_id == 0)
                return 0;

            glShaderSource(shader_id, 1, &source, nullptr);
            glCompileShader(shader_id);
            if (CheckShaderCompileStatus(shader_id) != GL_TRUE) {
                fprintf(stderr, "failed.\n");
                CheckShaderInfoLog(shader_id);
                glDeleteShader(shader_id);
                return 0;
            } else {
                fprintf(stderr, "successfull.\n");
                return shader_id;
            }
        }


        //-----------------------------------------------------------------------------
        // Name: CreateShaderFromFile()
        // Desc: 
        //-----------------------------------------------------------------------------
        GLuint CreateShaderFromFile(GLenum shader_type, const char* file_name, const char* preprocessor = nullptr) {
            char* fileContent = Tools::ReadFile(file_name);
            if (!fileContent) {
                fprintf(stderr, "Shader creation failed, input file is empty or missing!\n");
                return 0;
            }

            std::string shader_header;
            if (preprocessor) {
                shader_header += preprocessor;
            }
            if (Variables::Shader::UserTest) {
                shader_header += "#define USER_TEST\n";
            }
            
            std::string shader_source = fileContent;
            if (!shader_header.empty()) {
                std::size_t insertIdx = shader_source.find("\n", shader_source.find("#version"));
                shader_source.insert((insertIdx != std::string::npos) ? insertIdx : 0, std::string("\n") + shader_header + "\n\n");
            }

            delete[] fileContent;

            return CreateShaderFromSource(shader_type, shader_source.c_str());
        }


        //-----------------------------------------------------------------------------
        // Name: _updateProgramList()
        // Desc: 
        //-----------------------------------------------------------------------------
        void _updateProgramList(GLuint oldProgram, GLuint newProgram) {
            // Remove program from OpenGL and internal list
            for (std::vector<OpenGL::Program>::iterator it = OpenGL::programs.begin(); it != OpenGL::programs.end(); ++it) {
                if (it->id == oldProgram) {
                    OpenGL::programs.erase(it);
                     break;
                }
            }
   
            // Save program ID if it contains user variables
            if ((glGetUniformLocation(newProgram, "u_UserVariableInt") > -1) || 
                (glGetUniformLocation(newProgram, "u_UserVariableFloat") > -1) ||
                (glGetUniformLocation(newProgram, "u_FrameCounter") > -1) ||
                (glGetUniformLocation(newProgram, "u_MVPMatrix") > -1) || 
                (glGetUniformLocation(newProgram, "u_ModelViewMatrix") > -1) || 
                (glGetUniformLocation(newProgram, "u_ModelViewMatrixInv") > -1) || 
                (glGetUniformLocation(newProgram, "u_ProjectionMatrix") > -1) ||
                (glGetUniformLocation(newProgram, "u_Viewport") > -1) ||
                (glGetUniformLocation(newProgram, "u_ZOffset") > -1)) {
                OpenGL::programs.push_back(newProgram);
            }
        }


        //-----------------------------------------------------------------------------
        // Name: CreateShaderProgramFromSource()
        // Desc: 
        //-----------------------------------------------------------------------------
        bool CreateShaderProgramFromSource(GLuint& programId, GLint count, const GLenum* shader_types, const char** source) {
            if (!shader_types || !source)
                return false;

            // Create shader program object
            GLuint pr_id = glCreateProgram();
            for (int i = 0; i < count; i++) {
                GLuint shader_id = CreateShaderFromSource(shader_types[i], source[i]);
                if (shader_id == 0) {
                    glDeleteProgram(pr_id);
                    return false;
                }
                glAttachShader(pr_id, shader_id);
                glDeleteShader(shader_id);
            }
            glLinkProgram(pr_id);
            if (!CheckProgramLinkStatus(pr_id)) {
                CheckProgramInfoLog(pr_id);
                fprintf(stderr, "program linking failed.\n");
                fprintf(stderr, "-------------------------------------------------------------------------------\n");
                glDeleteProgram(pr_id);
                return false;
            }

            // Remove program from OpenGL and update internal list
            glDeleteProgram(programId);
            _updateProgramList(programId, pr_id);
            programId = pr_id;
   
            fprintf(stderr, "-------------------------------------------------------------------------------\n");
            
            return true;
        }


        //-----------------------------------------------------------------------------
        // Name: CreateShaderProgramFromFile()
        // Desc: 
        //-----------------------------------------------------------------------------
        bool CreateShaderProgramFromFile(GLuint& programId, const char* vs, const char* tc,
                                         const char* te, const char* gs, const char* fs, 
                                         const char* preprocessor = nullptr,
                                         const std::vector<char *> * tbx = nullptr) {
            GLenum shader_types[5] = {
                static_cast<GLenum>(vs ? GL_VERTEX_SHADER          : GL_NONE),
                static_cast<GLenum>(tc ? GL_TESS_CONTROL_SHADER    : GL_NONE),
                static_cast<GLenum>(te ? GL_TESS_EVALUATION_SHADER : GL_NONE),
                static_cast<GLenum>(gs ? GL_GEOMETRY_SHADER        : GL_NONE),
                static_cast<GLenum>(fs ? GL_FRAGMENT_SHADER        : GL_NONE),
            };
            const char* source_file_names[5] = {
                vs, tc, te, gs, fs
            };

            // Create shader program object
            GLuint pr_id = glCreateProgram();
            for (int i = 0; i < 5; i++) {
                if (source_file_names[i]) {
                    GLuint shader_id = CreateShaderFromFile(shader_types[i], source_file_names[i], preprocessor);
                    if (shader_id == 0) {
                        glDeleteProgram(pr_id);
                        return false;
                    }
                    glAttachShader(pr_id, shader_id);
                    glDeleteShader(shader_id);
                }
            }
            if (tbx && !tbx->empty()) {
                glTransformFeedbackVaryings(pr_id, tbx->size(), &(*tbx)[0], GL_INTERLEAVED_ATTRIBS);
            }
            glLinkProgram(pr_id);
            if (!CheckProgramLinkStatus(pr_id)) {
                CheckProgramInfoLog(pr_id);
                fprintf(stderr, "Program linking failed!\n");
                fprintf(stderr, "-------------------------------------------------------------------------------\n");
                glDeleteProgram(pr_id);
                return false;
            }

            // Remove program from OpenGL and update internal list
            glDeleteProgram(programId);
            _updateProgramList(programId, pr_id);
            programId = pr_id;

            fprintf(stderr, "-------------------------------------------------------------------------------\n");

            return true;
        }
    } // end of namespace Shader


    namespace Texture {
        //-----------------------------------------------------------------------------
        // Name: Show2DTexture()
        // Desc: 
        //-----------------------------------------------------------------------------
        inline void Show2DTexture(GLuint tex_id, GLint x, GLint y, GLsizei width, GLsizei height, int lod = 0, bool border = false) {
            if (glIsTexture(tex_id) == GL_FALSE) {
                return;
            }

            static const GLenum SHADER_TYPES[] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
            // Vertex shader
            static const char* vertex_shader = 
                "#version 400 core\n\
                uniform vec4 Vertex[4];\n\
                out     vec2 v_TexCoord;\n\
                void main(void) {\n\
                  v_TexCoord  = Vertex[gl_VertexID].zw;\n\
                  gl_Position = vec4(Vertex[gl_VertexID].xy, 0.0, 1.0f);\n\
                }";
            // Fragment shader GL_RGBA
            static const char* fragment_shader_rgba8 = 
                "#version 400 core\n\
                layout (location = 0) out vec4 FragColor;\n\
                in      vec2      v_TexCoord;\n\
                uniform sampler2D u_Texture;\n\
                uniform int       u_Lod = 0;\n\
                void main(void) {\n\
                  FragColor = vec4(textureLod(u_Texture, v_TexCoord, u_Lod).rgb, 1.0);\n\
                }";
            // Fragment shader GL_RGBA multisampled
            static const char* fragment_shader_rgba8_ms =
                "#version 400 core\n\
                layout (location = 0) out vec4 FragColor;\n\
                in      vec2        v_TexCoord;\n\
                uniform sampler2DMS u_Texture;\n\
                uniform int         u_NumSamples;\n\
                void main(void) {\n\
                  vec3 color = vec3(0.0);\n\
                  ivec2 texCoords = ivec2(v_TexCoord * textureSize(u_Texture));\n\
                  for (int i = 0; i < u_NumSamples; i++)\n\
                     color += texelFetch(u_Texture, texCoords, i).rgb;\n\
                  FragColor = vec4(color/u_NumSamples, 1.0);\n\
                }";
            // Fragment shader GL_R32UI
           static const char* fragment_shader_r32ui =
               "#version 400 core\n\
                layout (location = 0) out vec4 FragColor;\n\
                in      vec2       v_TexCoord;\n\
                uniform usampler2D u_Texture;\n\
                uniform int        u_Lod = 0;\n\
                void main(void) {\n\
                   uint color = textureLod(u_Texture, v_TexCoord, u_Lod).r;\n\
                   FragColor = vec4(float(color) * 0.125, 0.0, 0.0, 1.0);\n\
                }";
            static GLuint s_program_ids[3] = {0};

            GLint num_samples   = 1;
            GLint tex_format    = GL_RGBA8;
            GLint tex_comp_mode = GL_NONE;
            glGetTextureLevelParameteriv(tex_id, 0, GL_TEXTURE_SAMPLES, &num_samples);
            glGetTextureLevelParameteriv(tex_id, 0, GL_TEXTURE_INTERNAL_FORMAT, &tex_format);
            // GLsizei tex_width = 0; glGetTextureLevelParameteriv(tex_id, 0, GL_TEXTURE_WIDTH, &tex_width);
            // GLsizei tex_height = 0; glGetTextureLevelParameteriv(tex_id, 0, GL_TEXTURE_HEIGHT, &tex_height);
            GLenum target = (num_samples == 0) ? GL_TEXTURE_2D : GL_TEXTURE_2D_MULTISAMPLE;
            if (target == GL_TEXTURE_2D)
            {
                glGetTextureParameteriv(tex_id, GL_TEXTURE_COMPARE_MODE, &tex_comp_mode);
                glTextureParameteri(tex_id, GL_TEXTURE_COMPARE_MODE, GL_NONE);   // disable compare mode
            }

            // Setup texture
            GLint originalTexId = 0;
            glActiveTexture(GL_TEXTURE0);
            glGetIntegerv(GL_TEXTURE_BINDING_2D, &originalTexId);
            glBindTexture(GL_TEXTURE_2D, tex_id);

            glm::vec4 vp;
            glGetFloatv(GL_VIEWPORT, &vp.x);
            glm::vec2 tcOffse = border ? glm::vec2(1.0f / width, 1.0f / height) : glm::vec2(0.0f);
            const GLfloat normalized_coords_with_tex_coords[] = {
                        (x - vp.x) / (vp.z - vp.x)*2.0f - 1.0f,          (y - vp.y) / (vp.w - vp.y)*2.0f - 1.0f, 0.0f - tcOffse.x, 0.0f - tcOffse.y,
                (x + width - vp.x) / (vp.z - vp.x)*2.0f - 1.0f,          (y - vp.y) / (vp.w - vp.y)*2.0f - 1.0f, 1.0f + tcOffse.x, 0.0f - tcOffse.y,
                (x + width - vp.x) / (vp.z - vp.x)*2.0f - 1.0f, (y + height - vp.y) / (vp.w - vp.y)*2.0f - 1.0f, 1.0f + tcOffse.x, 1.0f + tcOffse.y,
                        (x - vp.x) / (vp.z - vp.x)*2.0f - 1.0f, (y + height - vp.y) / (vp.w - vp.y)*2.0f - 1.0f, 0.0f - tcOffse.x, 1.0f + tcOffse.y,
            };

            // Compile shaders
            const char* sources[2] = {
                vertex_shader, (tex_format == GL_R32UI) ? fragment_shader_r32ui : 
                ( (target == GL_TEXTURE_2D) ? fragment_shader_rgba8 : fragment_shader_rgba8_ms )
            };
            int index = (tex_format == GL_R32UI) ? 2 : ((target == GL_TEXTURE_2D) ? 0 : 1);
            if (s_program_ids[index] == 0) {
                if (!Shader::CreateShaderProgramFromSource(s_program_ids[index], 2, SHADER_TYPES, sources)) {
                    fprintf(stderr, "Show2DTexture: Unable to compile shader program.");
                    return;
                }
            }
            GLint current_program_id = 0;
            glGetIntegerv(GL_CURRENT_PROGRAM, &current_program_id);
            GLboolean depth_test_enabled = glIsEnabled(GL_DEPTH_TEST);

            // Render textured screen quad
            glDisable(GL_DEPTH_TEST);
            glUseProgram(s_program_ids[index]);
            glUniform1i(glGetUniformLocation(s_program_ids[index], "u_Texture"), 0);
            glUniform1i(glGetUniformLocation(s_program_ids[index], "u_NumSamples"), num_samples);
            glUniform1i(glGetUniformLocation(s_program_ids[index], "u_Lod"), lod);
            glUniform4fv(glGetUniformLocation(s_program_ids[index], "Vertex"), 4, normalized_coords_with_tex_coords);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            glUseProgram(current_program_id);
            if (depth_test_enabled)
                glEnable(GL_DEPTH_TEST);
            if (target == GL_TEXTURE_2D)
                glTextureParameteri(tex_id, GL_TEXTURE_COMPARE_MODE, tex_comp_mode);   // set original compare mode
            glBindTexture(GL_TEXTURE_2D, originalTexId);
        }


        //-----------------------------------------------------------------------------
        // Name: ShowCubeTexture()
        // Desc: 
        //-----------------------------------------------------------------------------
        inline void ShowCubeTexture(GLuint tex_id, GLint x, GLint y, GLsizei width, GLsizei height) {
            if (tex_id == 0) return;

            static const GLenum SHADER_TYPES[] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
            static const char* sources[2] = {
                // Vertex shader
                "#version 400 core\n\
                uniform vec3 u_Vertices[36];\n\
                uniform vec3 u_TexCoords[36];\n\
                uniform vec2 u_Offset[7];\n\
                uniform vec4 u_Scale;\n\
                out vec3 v_TexCoord;\n\
                void main(void) {\n\
                    vec3 vertex = u_Vertices[gl_VertexID];\n\
                    v_TexCoord  = u_TexCoords[gl_VertexID]; \n\
                    gl_Position = vec4((vertex.xy*u_Scale.xy + u_Offset[int(vertex.z)]) * u_Scale.zw - 1.0, 0.0, 1.0f);\n\
                }",
                // Fragment shader
                "#version 400 core\n\
                layout (location = 0) out vec4 FragColor;    \n\
                uniform samplerCube u_Texture;\n\
                uniform vec4 u_Color;\n\
                in vec3 v_TexCoord; \n\
                void main(void) {\n\
                    FragColor = vec4(texture(u_Texture, v_TexCoord).rgb, 1.0) * u_Color;\n\
                }"
            };
            static GLuint s_program_id = 0;
            if (s_program_id == 0) {
                if (!Shader::CreateShaderProgramFromSource(s_program_id, 2, SHADER_TYPES, sources)) {
                    fprintf(stderr, "ShowCubeTexture: Unable to compile shader program.");
                    return;
                }

                // Setup static uniforms
                const GLfloat vertices[] = {
                    0.0f, 0.0f, 1.0f,   1.0f, 0.0f, 1.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f, 1.0f,   // positive x
                    0.0f, 0.0f, 2.0f,   1.0f, 0.0f, 2.0f,   1.0f, 1.0f, 2.0f,   0.0f, 0.0f, 2.0f,   1.0f, 1.0f, 2.0f,   0.0f, 1.0f, 2.0f,   // negative x
                    0.0f, 0.0f, 3.0f,   1.0f, 0.0f, 3.0f,   1.0f, 1.0f, 3.0f,   0.0f, 0.0f, 3.0f,   1.0f, 1.0f, 3.0f,   0.0f, 1.0f, 3.0f,   // positive y
                    0.0f, 0.0f, 4.0f,   1.0f, 0.0f, 4.0f,   1.0f, 1.0f, 4.0f,   0.0f, 0.0f, 4.0f,   1.0f, 1.0f, 4.0f,   0.0f, 1.0f, 4.0f,   // negative y
                    0.0f, 0.0f, 5.0f,   1.0f, 0.0f, 5.0f,   1.0f, 1.0f, 5.0f,   0.0f, 0.0f, 5.0f,   1.0f, 1.0f, 5.0f,   0.0f, 1.0f, 5.0f,   // positive z
                    0.0f, 0.0f, 6.0f,   1.0f, 0.0f, 6.0f,   1.0f, 1.0f, 6.0f,   0.0f, 0.0f, 6.0f,   1.0f, 1.0f, 6.0f,   0.0f, 1.0f, 6.0f,   // negative z
                };
                const GLfloat texCoords[] = {
                     1.0f,-1.0f, 1.0f,   1.0f,-1.0f,-1.0f,   1.0f, 1.0f,-1.0f,   1.0f,-1.0f, 1.0f,   1.0f, 1.0f,-1.0f,   1.0f, 1.0f, 1.0f,  // positive x
                    -1.0f,-1.0f,-1.0f,  -1.0f,-1.0f, 1.0f,  -1.0f, 1.0f, 1.0f,  -1.0f,-1.0f,-1.0f,  -1.0f, 1.0f, 1.0f,  -1.0f, 1.0f,-1.0f,  // negative x
                     1.0f, 1.0f,-1.0f,  -1.0f, 1.0f,-1.0f,  -1.0f, 1.0f, 1.0f,   1.0f, 1.0f,-1.0f,  -1.0f, 1.0f, 1.0f,   1.0f, 1.0f, 1.0f,  // positive y
                     1.0f,-1.0f, 1.0f,  -1.0f,-1.0f, 1.0f,  -1.0f,-1.0f,-1.0f,   1.0f,-1.0f, 1.0f,  -1.0f,-1.0f,-1.0f,   1.0f,-1.0f,-1.0f,  // negative y
                    -1.0f,-1.0f, 1.0f,   1.0f,-1.0f, 1.0f,   1.0f, 1.0f, 1.0f,  -1.0f,-1.0f, 1.0f,   1.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 1.0f,  // positive z
                     1.0f,-1.0f,-1.0f,  -1.0f,-1.0f,-1.0f,  -1.0f, 1.0f,-1.0f,   1.0f,-1.0f,-1.0f,  -1.0f, 1.0f,-1.0f,   1.0f, 1.0f,-1.0f,  // negative z
                };

                glProgramUniform1i(s_program_id, glGetUniformLocation(s_program_id, "u_Texture"), 0);
                glProgramUniform4f(s_program_id, glGetUniformLocation(s_program_id, "u_Color"), 1.0f, 1.0f, 1.0f, 1.0f);
                glProgramUniform3fv(s_program_id, glGetUniformLocation(s_program_id, "u_Vertices"), 36, vertices);
                glProgramUniform3fv(s_program_id, glGetUniformLocation(s_program_id, "u_TexCoords"), 36, texCoords);
            }

            glm::vec4 vp;
            glGetFloatv(GL_VIEWPORT, &vp.x);
            const GLfloat offset[] = {
                (GLfloat)x, (GLfloat)y, 
                (GLfloat)x, y+height/3.0f,          // positive_x
                x+width/2.0f, y+height/3.0f,        // negative_x
                x+width/4.0f, y+height*2.0f/3.0f,   // positive_y
                x+width/4.0f, (GLfloat)y,           // negative_y
                x+width*3.0f/4.0f, y+height/3.0f,   // positive_z
                x+width/4.0f, y+height/3.0f,        // negative_z
            };

            GLint current_program_id = 0;
            glGetIntegerv(GL_CURRENT_PROGRAM, &current_program_id);
            GLboolean depth_test_enabled = glIsEnabled(GL_DEPTH_TEST);
            GLboolean blending_enabled   = glIsEnabled(GL_BLEND);

            // Render textured screen quad
            glDisable(GL_DEPTH_TEST);
            glUseProgram(s_program_id);
            glBindTextureUnit(0, tex_id);
            glUniform2fv(glGetUniformLocation(s_program_id, "u_Offset"), 7, offset);
            glUniform4f(glGetUniformLocation(s_program_id, "u_Scale"), width/4.0f, height/3.0f, 2.0f/(vp.z - vp.x), 2.0f/(vp.w - vp.y));
            glDrawArrays(GL_TRIANGLES, 0, 36);

            glUseProgram(current_program_id);
            if (depth_test_enabled)
                glEnable(GL_DEPTH_TEST);
            if (blending_enabled)
                glEnable(GL_BLEND);
        }


        //-----------------------------------------------------------------------------
        // Name: LoadR8()
        // Desc: 
        //-----------------------------------------------------------------------------
        GLuint LoadR8(const char* filename, GLsizei* num_texels = nullptr) {
            // Create mipmapped texture with image
            size_t bytes_read = 0;
            const char* rgb_data = Tools::ReadFile(filename, &bytes_read);
            if (!rgb_data)
                return 0;

            const GLsizei width = static_cast<GLsizei>(sqrtf((float)bytes_read));
            assert(width*width == bytes_read);
            GLsizei max_level = glm::log2(width) + 1;
            
            GLuint texId = 0;
            glCreateTextures(GL_TEXTURE_2D, 1, &texId);
            glTextureParameteri(texId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTextureParameteri(texId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(texId, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTextureParameteri(texId, GL_TEXTURE_WRAP_T, GL_CLAMP);
            glTextureStorage2D(texId, max_level, GL_R8, width, width);
            glTextureSubImage2D(texId, 0, 0, 0, width, width, GL_RED, GL_UNSIGNED_BYTE, rgb_data);
            glGenerateTextureMipmap(texId);

            delete [] rgb_data;

            if (num_texels)
                *num_texels = width*width;

            return texId;
        }


        //-----------------------------------------------------------------------------
        // Name: LoadRGB8()
        // Desc: 
        //-----------------------------------------------------------------------------
        GLuint LoadRGB8(const char* filename, GLsizei* num_texels = nullptr) {
            // Create mipmapped texture from raw file
            size_t bytes_read = 0;
            const char* rgb_data = Tools::ReadFile(filename, &bytes_read);
            if (!rgb_data)
                return 0;

            const GLsizei width = static_cast<GLsizei>(sqrtf(bytes_read / 3.0f));
            assert(width*width*3 == bytes_read);
            GLsizei max_level = glm::log2(width) + 1;

            GLuint texId = 0;
            glCreateTextures(GL_TEXTURE_2D, 1, &texId);
            glTextureParameteri(texId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTextureParameteri(texId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(texId, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTextureParameteri(texId, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTextureStorage2D(texId, max_level, GL_RGBA8, width, width);
            glTextureSubImage2D(texId, 0, 0, 0, width, width, GL_RGB, GL_UNSIGNED_BYTE, rgb_data);
            glGenerateTextureMipmap(texId);

            delete [] rgb_data;

            if (num_texels)
                *num_texels = width*width;

            return texId;
        }


        //-----------------------------------------------------------------------------
        // Name: LoadRGBA8()
        // Desc: resolution = (width, height, mipmap levels)
        //-----------------------------------------------------------------------------
        inline GLuint LoadRGB8(const char* filename, glm::ivec3 resolution) {
            // Create mipmapped texture from raw file
            size_t bytes_read = 0;
            const char* rgb_data = Tools::ReadFile(filename, &bytes_read);
 
            if (!rgb_data)
                return 0;

            GLuint texId = 0;
            glGenTextures(1, &texId);
            glCreateTextures(GL_TEXTURE_2D, 1, &texId);
            glTextureParameteri(texId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTextureParameteri(texId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(texId, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTextureParameteri(texId, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTextureStorage2D(texId, resolution.z, GL_RGBA8, resolution.x, resolution.y);
            
            GLint level = 0;
            const char* ptr = rgb_data; 
            while ((bytes_read > 0) && (level < resolution.z)) {
                glTextureSubImage2D(texId, level, 0, 0, resolution.x, resolution.y, GL_RGB, GL_UNSIGNED_BYTE, ptr);

                const size_t bytes_to_move = 3 * resolution.x * resolution.y * sizeof(GLubyte);
                ptr += bytes_to_move;
                bytes_read -= bytes_to_move;

                if (resolution.x > 1)
                    resolution.x >>= 1;
                if (resolution.y > 1)
                    resolution.y >>= 1;
                level++;
            }
            glTextureParameteri(texId, GL_TEXTURE_MAX_LEVEL, level - 1);

            delete [] rgb_data;
			assert(glGetError() == GL_NO_ERROR);
            return texId;
        }


        //-----------------------------------------------------------------------------
        // Name: CreateColoredTexture()
        // Desc: 
        //-----------------------------------------------------------------------------
        GLuint CreateColoredTexture(GLuint texture_id) {
            if (texture_id == 0) 
                return 0;

            // Clone texture with colorized mipmap layers
            GLint width = 0, height = 0, max_level = 0;
            glGetTextureLevelParameteriv(texture_id, 0, GL_TEXTURE_WIDTH, &width);
            glGetTextureLevelParameteriv(texture_id, 0, GL_TEXTURE_HEIGHT, &height);
            glGetTextureParameteriv(texture_id, GL_TEXTURE_IMMUTABLE_LEVELS, &max_level);
            if (width*height == 0)
                return 0;

            GLuint colored_texture_id = 0;
            glCreateTextures(GL_TEXTURE_2D, 1, &colored_texture_id);
            glTextureParameteri(colored_texture_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTextureParameteri(colored_texture_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(colored_texture_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTextureParameteri(colored_texture_id, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTextureStorage2D(colored_texture_id, max_level, GL_RGBA8, width, height);

            GLubyte* texels = new GLubyte[width * height * 4];
            const GLubyte color_shift[10][3] = {{0, 6, 6}, {0, 0, 6}, {6, 0, 6}, {6, 0, 0}, {6, 6, 0}, {0, 6, 0}, {3, 3, 3}, {6, 6, 6}, {3, 6, 3}, {6, 3, 3} };
            const int MAX_COLOR_LEVELS = sizeof(color_shift) / sizeof(color_shift[0]);

            for (int level = 0; level < max_level; level++) {
                glGetTextureLevelParameteriv(texture_id, level, GL_TEXTURE_WIDTH, &width);
                glGetTextureLevelParameteriv(texture_id, level, GL_TEXTURE_HEIGHT, &height);
                glGetTextureImage(texture_id, level, GL_RGBA, GL_UNSIGNED_BYTE, width * height * 4 * sizeof(GLubyte), texels);

                for (int i = 0; i <width*height; i++) {
                    const int shift_level = glm::min(level, MAX_COLOR_LEVELS);
                    texels[4*i+0] >>= color_shift[shift_level][0];
                    texels[4*i+1] >>= color_shift[shift_level][1];
                    texels[4*i+2] >>= color_shift[shift_level][2];
                }
                glTextureSubImage2D(colored_texture_id, level, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, texels);
            }

            assert(glGetError() == GL_NO_ERROR);
            delete [] texels;
            return colored_texture_id;
        }


        //-----------------------------------------------------------------------------
        // Name: CreateSimpleTexture()
        // Desc: 
        //-----------------------------------------------------------------------------
        GLuint CreateSimpleTexture(GLint width, GLint height)
        {
            if (width * height <= 0) {
                return 0;
            }

            GLubyte* data = new GLubyte[width*height*4];
            assert(data);

            const GLubyte pattern_width = 16;
            GLubyte* ptr = data; 
            GLubyte pattern = 0xFF;

            for (int h = 0; h < height; h++) {              
                for (int w = 0; w < width; w++) {
                    *ptr++ = pattern; 
                    *ptr++ = pattern;
                    *ptr++ = pattern;
                    *ptr++ = 255;
                    if (w % pattern_width == 0)
                        pattern = ~pattern;
                }

                if (h % pattern_width == 0)
                    pattern = ~pattern;
            }

            glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 

            GLsizei max_level = glm::log2(glm::max(width, height)) + 1;

            GLuint tex_id = 0;
            glCreateTextures(GL_TEXTURE_2D, 1, &tex_id);
            glTextureParameteri(tex_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTextureParameteri(tex_id, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTextureParameteri(tex_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(tex_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTextureStorage2D(tex_id, max_level, GL_RGBA8, width, height);
            glTextureSubImage2D(tex_id, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateTextureMipmap(tex_id);

            delete [] data;

            return tex_id;
        } 
    }; // end of namespace Texture

    namespace Noise {
        /* Taken from "Lode's Computer Graphics Tutorial", http://lodev.org/cgtutor/randomnoise.html */

        const int NoiseWidth  = 128;
        const int NoiseHeight = 128;
        float Noise[NoiseHeight][NoiseWidth] = {0}; // The noise array


        //-----------------------------------------------------------------------------
        // Name: GenerateNoise()
        // Desc: Generates array of random values
        //-----------------------------------------------------------------------------
          void GenerateNoise() {
            if (Noise[0][0] != 0.0f)
                return;

            for (int y = 0; y < NoiseHeight; y++) {
                for (int x = 0; x < NoiseWidth; x++) {
                    Noise[y][x] = glm::compRand1<float>();
                }
            }
        }


        //-----------------------------------------------------------------------------
        // Name: SmoothNoise()
        // Desc: 
        //-----------------------------------------------------------------------------
        float SmoothNoise(float x, float y) {
           // Get fractional part of x and y
           const float fractX = x - int(x);
           const float fractY = y - int(y);

           // Wrap around
           const int x1 = (int(x) + NoiseWidth) % NoiseWidth;
           const int y1 = (int(y) + NoiseHeight) % NoiseHeight;

           // Neighbor values
           const int x2 = (x1 + NoiseWidth - 1) % NoiseWidth;
           const int y2 = (y1 + NoiseHeight - 1) % NoiseHeight;

           // Smooth the noise with bilinear interpolation
           return fractY * (fractX * Noise[y1][x1] + (1 - fractX) * Noise[y1][x2]) +
                  (1 - fractY) * (fractX * Noise[y2][x1] + (1 - fractX) * Noise[y2][x2]);
        }


        //-----------------------------------------------------------------------------
        // Name: Turbulence()
        // Desc: 
        //-----------------------------------------------------------------------------
        float Turbulence(int x, int y, int size) {
            const int initialSize = size;
            float value = 0.0;

            while(size >= 1) {
                const float sizeInv = 1.0f / size;
                value += SmoothNoise(x * sizeInv, y * sizeInv) * size;
                size >>= 1;
            }

            return(128.0f * value / initialSize);
        }


        //-----------------------------------------------------------------------------
        // Name: GenerateMarblePatter()
        // Desc: 
        //-----------------------------------------------------------------------------
        glm::u8vec3* GenerateMarblePatter(int width, int height, int turbSize = 32, float turbPower = 5.0f,
                                          float xPeriod = 5.0f, float yPeriod = 10.0f) {
            GenerateNoise();

            // xPeriod and yPeriod together define the angle of the lines
            // xPeriod and yPeriod both 0 ==> it becomes a normal clouds or turbulence pattern
            xPeriod /= NoiseWidth;  // defines repetition of marble lines in x direction
            yPeriod /= NoiseHeight; // defines repetition of marble lines in y direction

            glm::u8vec3* pixels = new glm::u8vec3[width * height];

            for(int y = 0; y < height; y++) {
                for(int x = 0; x < width; x++) {
                    const float xyValue = x * xPeriod + y * yPeriod + turbPower * Turbulence(x, y, turbSize) * 0.01227184f;
                    const float sineValue = 256 * fabs(sin(xyValue));
                    pixels[x + y*width] = glm::u8vec3(sineValue);
                }
                fprintf(stderr, "\rGenerateMarblePatter() - row %d generated\t\t", y);
            }

            return pixels;
        }


    }; // end of namespace Noise
}; // end of namespace Tools
