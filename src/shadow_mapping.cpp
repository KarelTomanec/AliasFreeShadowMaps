//-----------------------------------------------------------------------------
//  Alias Free Shadow Mapping
//  31/01/2021
//-----------------------------------------------------------------------------
//  Controls:
//    [i/I]   ... inc/dec value of user integer variable
//    [f/F]   ... inc/dec value of user floating-point variable
//    [z/Z]   ... move scene along z-axis
//    [s]     ... show/hide first depth map texture
//    [d/D]   ... change depth map resolution
//    [l/L]   ... inc/dec light distance from the scene
//    [c]     ... compile shaders
//    [mouse] ... scene rotation (left button)
//-----------------------------------------------------------------------------
#include <iostream>
#include <limits>
#include "common.h"

// GLOBAL CONSTANTS____________________________________________________________
const char* TEXTURE_FILE_NAME = "../shared/textures/metal01.raw";
enum eTextureType { Diffuse = 0, DepthMap, ZBuffer, ZBufferShadow, VisibilityMap, HeadPointerImage, ListBuffer, ShadowMap, LightingMap, NumTextureTypes };

enum eAlgorithmPass {
    DepthTextureGeneration = 0,
    ShadowTest,
    ShadowTestAliasFree,
    RenderScene,
    VisibilityMapGeneration,
    ListBufferGeneration,
    NumPasses
};

// GLOBAL VARIABLES__________________________________________________________________________________________________________________
bool      g_ShowDepthTexture          = true;  // Show/hide depth texture
GLint     g_Resolution                = 1024;  // FBO size in pixels
GLuint    g_Textures[NumTextureTypes] =  {0};  // Textures - color (diffuse), depth map, z-buffer, z-buffer for hw shadow map
GLuint    g_ShadowTestSampler         =    0;  // Texture sampler for automatic shadow test
GLuint    g_Framebuffer               =    0;  // Virtual frame-buffer FBO id
GLuint    g_ShadowTestFramebuffer = 0;

GLuint    g_ProgramId[NumPasses]      =  {0};  // shader program IDs

GLint     g_ShadowMapsAlgo = 0; // The index of the currently running algorithm
bool      g_Switch     = false; // Variable saving algorithm switching.

GLuint list_buf; // Index of the list buffer
GLuint atomic_counter_buffer; // Index of the atomic counter buffer

GLuint q[2] = { 0 };
Tools::GPUTimer g_Timer[4];

// TRANSFORMATIONS___________________________________________________________________________________________________________________

glm::mat4& g_CameraViewMatrix       = Variables::Transform.ModelView;  // Camera view transformation ~ scene transformation
glm::mat4& g_CameraProjectionMatrix = Variables::Transform.Projection; // Camera projection transformation ~ scene projection
glm::vec3  g_LightPosition          = glm::vec3(0.0f, 20.0f, 0.0f);    // Light orientation 
glm::mat4  g_LightViewMatrix;                                          // Light view transformation
glm::mat4  g_LightProjectionMatrix  = glm::frustum(-1.0f, 1.0f,-1.0f, 1.0f, 1.0f, 1000.0f);   // Light projection transformation


static const GLfloat RECTANGLE[]{
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f
};

// FORWARD DECLARATIONS______________________________________________________________________________________________________________

/// <summary>
/// Main render loop.
/// </summary>
void display();

/// <summary>
/// Routine of the standard shadow mapping algorithm.
/// </summary>
void standardShadowMapping();

/// <summary>
/// Creates a virtual framebuffer that is used to create depth map from the perspective of the light.
/// </summary>
void createVirtualFramebuffer();

/// <summary>
/// Routine of the alias-free shadow maps algorithm.
/// </summary>
void aliasFreeShadowMapping();

/// <summary>
/// Creates textures and a virtual framebuffer that are essential to the functionality of the alias free shadow maps algorithm.
/// </summary>
void createTexturesAndFramebuffer();

/// <summary>
/// Updates lightView matrix based on the current position of the light.
/// </summary>
void updateLightViewMatrix();

/// <summary>
/// Sets initial opengl state.
/// </summary>
void initGL();

/// <summary>
/// Draws a rectangle across the entire screen.
/// </summary>
void drawRectangle();

/// <summary>
/// Creates head pointer image.
/// </summary>
void createHeadPointerImage();

/// <summary>
/// Recreates textures that depend on the window resolution.
/// </summary>
/// <param name="resolution">Resolution of the window</param>
void resizeWindow(const glm::ivec2& resolution);

// IMPLEMENTATION____________________________________________________________________________________________________________________

void display() {

    // Update camera & light transformations
    updateLightViewMatrix();

    glQueryCounter(q[0], GL_TIMESTAMP);

    if (g_ShadowMapsAlgo)
    {
        aliasFreeShadowMapping();
    }
    else
    {
        standardShadowMapping();
    }

    glQueryCounter(q[1], GL_TIMESTAMP);

    GLuint64 start, stop;
    glGetQueryObjectui64v(q[0], GL_QUERY_RESULT, &start);
    glGetQueryObjectui64v(q[1], GL_QUERY_RESULT, &stop);
    printf("Total time [ms]: %f\n", (stop - start) / 1000000.0);
    if (g_ShadowMapsAlgo)
    {
        printf("1. Visibility map generation [ms]: %f\n", g_Timer[0].get() / 1000000.0);
        printf("2. List buffer generation [ms]:    %f\n", g_Timer[1].get() / 1000000.0);
        printf("3. Shadow test [ms]:               %f\n", g_Timer[2].get() / 1000000.0);
        printf("4. Render scene [ms]:              %f\n", g_Timer[3].get() / 1000000.0);
    }

}

void standardShadowMapping()
{
    // Create a frame-buffer object
    createVirtualFramebuffer();

    // DEPTH TEXTURE GENERATION -----------------------------------------------
    GLuint pid = g_ProgramId[DepthTextureGeneration];
    glUseProgram(pid);


    glBindFramebuffer(GL_FRAMEBUFFER, g_Framebuffer);
    glViewport(0, 0, g_Resolution, g_Resolution);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUniformMatrix4fv(1, 1, GL_FALSE, &g_LightProjectionMatrix[0][0]);
    glUniformMatrix4fv(0, 1, GL_FALSE, &g_LightViewMatrix[0][0]);

    //glCullFace(GL_FRONT);
    glPolygonOffset(4.0f, 4.0f);
    glEnable(GL_POLYGON_OFFSET_FILL); // GPU feature to get rid of self shadowing
    Tools::DrawScene();
    //glCullFace(GL_BACK);

    glViewport(0, 0, Variables::WindowSize.x, Variables::WindowSize.y);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // SHADOW GENERATION ------------------------------------------------------
    pid = g_ProgramId[ShadowTest];
    glUseProgram(pid);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindTextures(0, 4, g_Textures);

    static GLuint sampler = 0;
    if (sampler == 0)
    {
        glCreateSamplers(1, &sampler);
        glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glSamplerParameteri(sampler, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glSamplerParameteri(sampler, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        glBindSampler(3, sampler);
    }

    glUniformMatrix4fv(glGetUniformLocation(pid, "u_ProjectionMatrix"), 1, GL_FALSE, &g_CameraProjectionMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(pid, "u_ModelViewMatrix"), 1, GL_FALSE, &g_CameraViewMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(pid, "u_LightProjectionMatrix"), 1, GL_FALSE, &g_LightProjectionMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(pid, "u_LightViewMatrix"), 1, GL_FALSE, &g_LightViewMatrix[0][0]);
    const glm::vec4 light_position = (g_CameraViewMatrix * glm::inverse(g_LightViewMatrix)) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    glUniform4fv(glGetUniformLocation(pid, "u_LightPosition"), 1, &light_position.x);

    // Calculate transformation matrix 'shadowTransformMatrix' and pass it into shader
    glm::mat4 shadowTransformMatrix = glm::mat4(1.0f);
    glm::mat4 matScale = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.5)), glm::vec3(0.5)); // * 0.5 + 0.5
    shadowTransformMatrix = matScale * g_LightProjectionMatrix * g_LightViewMatrix;
    glUniformMatrix4fv(glGetUniformLocation(pid, "u_ShadowTransformMatrix"), 1, GL_FALSE, &shadowTransformMatrix[0][0]);

    Tools::DrawScene();
    glUseProgram(0);

    // Show depth maps
    if (g_ShowDepthTexture)
    {
        Tools::Texture::Show2DTexture(g_Textures[DepthMap], Variables::WindowSize.x - 200, Variables::WindowSize.y - 200, 200, 200);
        Tools::Texture::Show2DTexture(g_Textures[ZBuffer], Variables::WindowSize.x - 200, Variables::WindowSize.y - 400, 200, 200);
    }
}

void createVirtualFramebuffer()
{
    static GLint s_Resolution = 0;

    if (s_Resolution == g_Resolution && g_Switch == false) return;
        
    g_Switch = false;

    // delete textures from previous pass
    glDeleteTextures(NumTextureTypes - DepthMap, &g_Textures[DepthMap]);
    glDeleteFramebuffers(1, &g_Framebuffer);

    // create depth map
    glCreateTextures(GL_TEXTURE_2D, 1, &g_Textures[DepthMap]);
    glTextureParameteri(g_Textures[DepthMap], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(g_Textures[DepthMap], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureStorage2D(g_Textures[DepthMap], 1, GL_RGBA32F, g_Resolution, g_Resolution);

    // create z buffer - faster, but it can have issues. Z coord is not lineary interpolated -> perspective alias
    glCreateTextures(GL_TEXTURE_2D, 1, &g_Textures[ZBuffer]);
    glTextureParameteri(g_Textures[ZBuffer], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(g_Textures[ZBuffer], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureStorage2D(g_Textures[ZBuffer], 1, GL_DEPTH_COMPONENT32, g_Resolution, g_Resolution);

    // create framebuffer and attach textures to it
    glCreateFramebuffers(1, &g_Framebuffer);
    glNamedFramebufferTexture(g_Framebuffer, GL_COLOR_ATTACHMENT0, g_Textures[DepthMap], 0);
    glNamedFramebufferTexture(g_Framebuffer, GL_DEPTH_ATTACHMENT, g_Textures[ZBuffer], 0); 

    g_Textures[ZBufferShadow] = g_Textures[ZBuffer];

    // Check framebuffer status
    if (g_Framebuffer > 0)
    {
        GLenum const error = glGetError();
        GLenum const status = glCheckNamedFramebufferStatus(g_Framebuffer, GL_FRAMEBUFFER);

        if ((status == GL_FRAMEBUFFER_COMPLETE) && (error == GL_NO_ERROR))
            s_Resolution = g_Resolution;
        else
            printf("FBO creation failed, glCheckFramebufferStatus() = 0x%x, glGetError() = 0x%x\n", status, error);
    }
}

void aliasFreeShadowMapping()
{
    if (g_Switch)
    {
        resizeWindow(glm::ivec2(Variables::WindowSize.x, Variables::WindowSize.y));
        createHeadPointerImage();
        g_Switch = false;
    }
    else
    {
        createHeadPointerImage();
    }


    // CLEAR BUFFERS --------------------------------------------------------------

    GLuint* data;

    // Reset atomic counter
    GLuint zero = 0;
    glClearNamedBufferData(atomic_counter_buffer, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);

    // Clear head pointer image
    unsigned int clear_color0 = std::numeric_limits<unsigned int>::max();
    glClearTexImage(g_Textures[HeadPointerImage], 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &clear_color0);

    // Clear shadow map
    unsigned int clear_color1 = 0;
    glClearTexImage(g_Textures[ShadowMap], 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &clear_color1);


    // GENERATE VISIBILITY MAP ----------------------------------------------------

    g_Timer[0].start();

    GLuint pid = g_ProgramId[VisibilityMapGeneration];
    glUseProgram(pid);

    glBindFramebuffer(GL_FRAMEBUFFER, g_Framebuffer);
    GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);

    glBindTextureUnit(0, g_Textures[Diffuse]);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUniformMatrix4fv(0, 1, GL_FALSE, &g_CameraViewMatrix[0][0]);
    glUniformMatrix4fv(1, 1, GL_FALSE, &g_CameraProjectionMatrix[0][0]);
    glUniformMatrix4fv(2, 1, GL_FALSE, &g_LightViewMatrix[0][0]);

    const glm::vec4 light_position = (g_CameraViewMatrix * glm::inverse(g_LightViewMatrix)) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    glUniform4fv(3, 1, &light_position.x);

    Tools::DrawScene();

    g_Timer[0].stop();

    // GENERATE LIST BUFFER -------------------------------------------------------

    g_Timer[1].start();

    pid = g_ProgramId[ListBufferGeneration];
    glUseProgram(pid);

    glBindTextureUnit(1, g_Textures[VisibilityMap]);

    glDisable(GL_DEPTH_TEST);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    drawRectangle();

    g_Timer[1].stop();

    // SHADOW TEST ----------------------------------------------------------------
    //Tools::GPUTimer

    //if (query == 0) glCreateQueries(GL_TIME_ELAPSED, 1, &query);
    //glBeginQuery(GL_TIME_ELAPSED, query);


    g_Timer[2].start();

    glEnable(GL_CONSERVATIVE_RASTERIZATION_NV);

    pid = g_ProgramId[ShadowTestAliasFree];
    glUseProgram(pid);
    glUniformMatrix4fv(1, 1, GL_FALSE, &g_LightProjectionMatrix[0][0]);
    glUniformMatrix4fv(0, 1, GL_FALSE, &g_LightViewMatrix[0][0]);

    glBindTextureUnit(1, g_Textures[VisibilityMap]);
    glBindTextureUnit(2, g_Textures[HeadPointerImage]);

    glDisable(GL_CULL_FACE);
    glBindFramebuffer(GL_FRAMEBUFFER, g_ShadowTestFramebuffer);
    glViewport(0, 0, g_Resolution, g_Resolution);

    Tools::DrawScene();

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    glDisable(GL_CONSERVATIVE_RASTERIZATION_NV);


    g_Timer[2].stop();


    // RENDER SCENE ---------------------------------------------------------------


    g_Timer[3].start();

    pid = g_ProgramId[RenderScene];
    glUseProgram(pid);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, Variables::WindowSize.x, Variables::WindowSize.y);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawRectangle();


    g_Timer[0].stop();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // Show textures
    if (g_ShowDepthTexture)
    {
        Tools::Texture::Show2DTexture(g_Textures[VisibilityMap], Variables::WindowSize.x - 200, Variables::WindowSize.y - 200, 200, 200);
        Tools::Texture::Show2DTexture(g_Textures[HeadPointerImage], Variables::WindowSize.x - 200, Variables::WindowSize.y - 400, 200, 200);
        Tools::Texture::Show2DTexture(g_Textures[ShadowMap], Variables::WindowSize.x - 200, Variables::WindowSize.y - 600, 200, 200);
        Tools::Texture::Show2DTexture(g_Textures[LightingMap], Variables::WindowSize.x - 200, Variables::WindowSize.y - 800, 200, 200);
    }
}

void createHeadPointerImage()
{
    static GLint s_Resolution = 0;

    if (s_Resolution == g_Resolution && g_Switch == false)
        return;

    glDeleteTextures(1, &g_Textures[HeadPointerImage]);
    glDeleteFramebuffers(1, &g_ShadowTestFramebuffer);

    // Create head pointer texture
    glCreateTextures(GL_TEXTURE_2D, 1, &g_Textures[HeadPointerImage]);
    glTextureParameteri(g_Textures[HeadPointerImage], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(g_Textures[HeadPointerImage], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureStorage2D(g_Textures[HeadPointerImage], 1, GL_R32UI, g_Resolution, g_Resolution);
    glBindImageTexture(1, g_Textures[HeadPointerImage], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);


    glCreateFramebuffers(1, &g_ShadowTestFramebuffer);
    glNamedFramebufferParameteri(g_ShadowTestFramebuffer, GL_FRAMEBUFFER_DEFAULT_WIDTH, g_Resolution);
    glNamedFramebufferParameteri(g_ShadowTestFramebuffer, GL_FRAMEBUFFER_DEFAULT_HEIGHT, g_Resolution);

    s_Resolution = g_Resolution;
}

void resizeWindow(const glm::ivec2& resolution)
{
    if (g_ShadowMapsAlgo == 0) return;

    glDeleteTextures(1, &g_Textures[ZBuffer]);
    glDeleteTextures(1, &g_Textures[VisibilityMap]);
    glDeleteTextures(3, &g_Textures[ListBuffer]);
    glDeleteFramebuffers(1, &g_Framebuffer);
    glDeleteBuffers(1, &list_buf);

    // z buffer - faster, but it can have issues. Z coord is non lineary interpolated -> perspective alias
    glCreateTextures(GL_TEXTURE_2D, 1, &g_Textures[ZBuffer]);
    glTextureParameteri(g_Textures[ZBuffer], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(g_Textures[ZBuffer], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureStorage2D(g_Textures[ZBuffer], 1, GL_DEPTH_COMPONENT32F, resolution.x, resolution.y);

    // visibility map
    glCreateTextures(GL_TEXTURE_2D, 1, &g_Textures[VisibilityMap]);
    glTextureParameteri(g_Textures[VisibilityMap], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(g_Textures[VisibilityMap], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureStorage2D(g_Textures[VisibilityMap], 1, GL_RGBA32F, resolution.x, resolution.y);

    // lighting map
    glCreateTextures(GL_TEXTURE_2D, 1, &g_Textures[LightingMap]);
    glTextureParameteri(g_Textures[LightingMap], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(g_Textures[LightingMap], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureStorage2D(g_Textures[LightingMap], 1, GL_RGBA8, resolution.x, resolution.y);
    glBindImageTexture(4, g_Textures[LightingMap], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

    // Create the linked list storage buffer
    glCreateBuffers(1, &list_buf);
    glNamedBufferStorage(list_buf, resolution.x * resolution.y * sizeof(glm::uvec4), NULL, GL_NONE);

    // Bind it to a texture (for use as a TBO)
    glCreateTextures(GL_TEXTURE_BUFFER, 1, &g_Textures[ListBuffer]);
    glTextureBuffer(g_Textures[ListBuffer], GL_RGBA32UI, list_buf);
    glBindImageTexture(2, g_Textures[ListBuffer], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32UI);

    // Create shadow map
    glCreateTextures(GL_TEXTURE_2D, 1, &g_Textures[ShadowMap]);
    glTextureParameteri(g_Textures[ShadowMap], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(g_Textures[ShadowMap], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureStorage2D(g_Textures[ShadowMap], 1, GL_R32UI, resolution.x, resolution.y);
    glBindImageTexture(3, g_Textures[ShadowMap], 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

    // Create framebuffer
    glCreateFramebuffers(1, &g_Framebuffer);
    glNamedFramebufferTexture(g_Framebuffer, GL_DEPTH_ATTACHMENT, g_Textures[ZBuffer], 0);
    glNamedFramebufferTexture(g_Framebuffer, GL_COLOR_ATTACHMENT0, g_Textures[VisibilityMap], 0);
    glNamedFramebufferTexture(g_Framebuffer, GL_COLOR_ATTACHMENT1, g_Textures[LightingMap], 0);

    // Check framebuffer status
    if (g_Framebuffer > 0)
    {
        GLenum const error = glGetError();
        GLenum const status = glCheckNamedFramebufferStatus(g_Framebuffer, GL_FRAMEBUFFER);

        if (!((status == GL_FRAMEBUFFER_COMPLETE) && (error == GL_NO_ERROR)))
        {
            printf("FBO creation failed, glCheckFramebufferStatus() = 0x%x, glGetError() = 0x%x\n", status, error);
        }
    }
}

void initGL() {

    // Default scene distance
    Variables::Shader::SceneZOffset = 24.0f;

    // Set OpenGL state variables
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glViewport(0, 0, Variables::WindowSize.x, Variables::WindowSize.y);

    // Load a create texture for scene
    g_Textures[Diffuse] = Tools::Texture::LoadRGB8(TEXTURE_FILE_NAME);

    // Create the atomic counter buffer
    glCreateBuffers(1, &atomic_counter_buffer);
    glNamedBufferStorage(atomic_counter_buffer, sizeof(GLuint), NULL, GL_NONE);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomic_counter_buffer);

    glCreateQueries(GL_TIMESTAMP, 2, q);

    // Load shader program
    compileShaders();

}

void drawRectangle()
{
    static GLuint r_vao = 1;
    if (r_vao == 1)
    {
        glGenVertexArrays(1, &r_vao);
        glBindVertexArray(r_vao);
        GLuint vbo = 0;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(RECTANGLE), RECTANGLE, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (const void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
        glDeleteBuffers(1, &vbo);
    }

    glBindVertexArray(r_vao);
    const int NUM_VERTICES = sizeof(RECTANGLE) / (3 * sizeof(GLfloat));
    glDrawArrays(GL_TRIANGLES, 0, NUM_VERTICES);
    glBindVertexArray(0);
}


// Include GUI and control stuff
#include "controls.hpp"
