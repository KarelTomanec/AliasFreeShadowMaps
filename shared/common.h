//-----------------------------------------------------------------------------
//  [PGR2] Common function definitions
//  27/02/2008
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
#include <glew.h>
#ifdef _WIN32
#   define _CRT_SECURE_NO_WARNINGS
#   include <wglew.h>
#else
#   include <glxew.h>
#endif
#include <glfw3.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "implot/implot.h"
#include "implot/implot_internal.h"
#include <assert.h>
#include "./glm/glm.hpp"
#include "./glm/gtx/random.hpp"
#include "./glm/gtx/constants.hpp"
#include "./glm/gtc/matrix_transform.hpp"
#include "./glm/gtc/type_precision.hpp"
#include <chrono>
#include "tools.h"

// FUNCTION POINTER TYPES______________________________________________________
/* Function pointer types */
typedef void (* TInitGLCallback)(void);
typedef void (* TShowGUICallback)(void *);
typedef void (* TDisplayCallback)(void);
typedef void (* TReleaseOpenGLCallback)(void);
typedef void (* TWindowSizeChangedCallback)(const glm::ivec2&);
typedef void (* TMouseButtonChangedCallback)(int, int);
typedef void (* TMousePositionChangedCallback)(double, double);
typedef void (* TKeyboardChangedCallback)(int, int, int);

// FORWARD DECLARATIONS________________________________________________________
void compileShaders(void * = nullptr);

// INTERNAL CONSTANTS__________________________________________________________
#define PGR2_SHOW_MEMORY_STATISTICS 0x0F000001
#define PGR2_DISABLE_VSYNC          0x0F000002
#define PGR2_DISABLE_BUFFER_SWAP    0x0F000004

// INTERNAL USER CALLBACK FUNCTION POINTERS____________________________________
namespace Callbacks {
    namespace User {
        TReleaseOpenGLCallback        OpenGLRelease = nullptr;
        TDisplayCallback              Display = nullptr;
        TShowGUICallback              ShowGUI = nullptr;
        TWindowSizeChangedCallback    WindowSizeChanged = nullptr;
        TMouseButtonChangedCallback   MouseButtonChanged = nullptr;
        TMousePositionChangedCallback MousePositionChanged = nullptr;
        TKeyboardChangedCallback      KeyboardChanged = nullptr;
    } // end of namespace User

    namespace GUI {
        //-----------------------------------------------------------------------------
        // Name: Set()
        // Desc: Default GUI "set" callback
        //-----------------------------------------------------------------------------
        template <typename T> void Set(const void* value, void* clientData) {
            *static_cast<T*>(clientData) = *static_cast<const T*>(value);
        }
        //-----------------------------------------------------------------------------
        // Name: Set()
        // Desc: Default GUI "set" callback
        //-----------------------------------------------------------------------------
        template <typename T> void SetCompile(const void* value, void* clientData) {
            *static_cast<T*>(clientData) = *static_cast<const T*>(value);
            compileShaders();
        }
        //-----------------------------------------------------------------------------
        // Name: Get()
        // Desc: Default GUI "get" callback
        //-----------------------------------------------------------------------------
        template <typename T> void Get(void* value, void* clientData) {
            *static_cast<T*>(value) = *static_cast<T*>(clientData);
        }

        //-----------------------------------------------------------------------------
        // Name: Show()
        // Desc: 
        //-----------------------------------------------------------------------------
        void Show(void * user) {
            const int   oneInt   = 1;
            const float oneFloat = 0.01f;

            if (Variables::Debug) {
                glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE);
                glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE, 0, nullptr, GL_TRUE);
                assert(glGetError() == GL_NO_ERROR);
            }

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Once);
            ImGui::GetStyle().Alpha = 0.80f;
            ImGui::GetStyle().Colors[ImGuiCol_PlotHistogram] = ImVec4(0.19f, 0.42f, 0.67f, 1.0f);
           
            // Append example controls
            int statisticWindowPos = 255;
            if (User::ShowGUI)
                User::ShowGUI( &statisticWindowPos);

            ImGui::Begin("Controls");
            if (!User::ShowGUI)
                ImGui::SetWindowSize(ImVec2(220, statisticWindowPos - 15), ImGuiCond_Once);
            if (ImGui::CollapsingHeader("Shader", ImGuiTreeNodeFlags_DefaultOpen)) {
                if (ImGui::Checkbox("USER_TEST", &Variables::Shader::UserTest))
                    compileShaders();
                ImGui::SameLine(105.0f, 0.0f);

                if (ImGui::Button("compile shaders"))
                    compileShaders();

                ImGui::InputScalar("int", ImGuiDataType_S32, &Variables::Shader::Int, &oneInt);
                // ImGui::SliderInt("int", &Variables::Shader::Int, -10000, 10000, "%d");
                ImGui::SliderFloat("float", &Variables::Shader::Float, 0.0f, 1.0f, "%.3f");
            }

            bool hasSceneControl = false;
            for (unsigned int i = 0; i < OpenGL::programs.size(); i++) {
                const OpenGL::Program& program = OpenGL::programs[i];
                if (program.hasMVMatrix() || program.hasZOffset()) {
                    hasSceneControl = true;
                    break;
                }
            }

            if (hasSceneControl  && ImGui::CollapsingHeader("Scene", ImGuiTreeNodeFlags_DefaultOpen)) {
                for (unsigned int i = 0; i < OpenGL::programs.size(); i++) {
                    const OpenGL::Program& program = OpenGL::programs[i];
                    if (program.hasMVMatrix() || program.hasZOffset()) {
                        ImGui::SliderFloat("z-offset", &Variables::Shader::SceneZOffset, 0.0f, 100.0f);
                        break;
                    }
                }
            }

            if (ImGui::CollapsingHeader("User variables", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::InputScalar("int0", ImGuiDataType_S32, &Variables::Menu::Int0, &oneInt);
                ImGui::InputScalar("int1", ImGuiDataType_S32, &Variables::Menu::Int1, &oneInt);
                ImGui::InputScalar("float0", ImGuiDataType_Float, &Variables::Menu::Float0, &oneFloat);
                ImGui::InputScalar("float1", ImGuiDataType_Float, &Variables::Menu::Float1, &oneFloat);
            }
            ImGui::End();

            // Create statistic window
            ImGui::SetNextWindowPos(ImVec2(10, statisticWindowPos), ImGuiCond_Once);
            ImGui::SetNextWindowSize(ImVec2(220, 245), ImGuiCond_Once);
            ImGui::Begin("Statistic");
            ImGui::Text("Frame %d, FPS %.3f", Statistic::Frame::ID, Statistic::FPS);

            // GPU time
            static float gpu_times[50]    = {};
            static int   gpu_times_offset = 0;
            static int   gpu_max_time     = 0;
            static int   gpu_max_time_200 = 0;
            if (gpu_max_time < Statistic::Frame::GPUTime) gpu_max_time = Statistic::Frame::GPUTime;
            if (gpu_max_time_200 < Statistic::Frame::GPUTime) gpu_max_time_200 = Statistic::Frame::GPUTime;
            if (Statistic::Frame::ID % 200 == 0) {
                gpu_max_time     = gpu_max_time_200;
                gpu_max_time_200 = 0;
            }
            gpu_times[gpu_times_offset] = Statistic::Frame::GPUTime;
            gpu_times_offset = (gpu_times_offset + 1) % IM_ARRAYSIZE(gpu_times);
            char overlay[32];
            sprintf(overlay, "GPU time: %d", Statistic::Frame::GPUTime);
            ImGui::PlotLines("", gpu_times, IM_ARRAYSIZE(gpu_times), gpu_times_offset, overlay, 0.0f, gpu_max_time, ImVec2(205, 50));

            // CPU time
            static float cpu_times[50] = {};
            static int   cpu_times_offset = 0;
            static int   cpu_max_time = 0;
            static int   cpu_max_time_200 = 0;
            if (cpu_max_time < Statistic::Frame::CPUTime) cpu_max_time = Statistic::Frame::CPUTime;
            if (cpu_max_time_200 < Statistic::Frame::CPUTime) cpu_max_time_200 = Statistic::Frame::CPUTime;
            if (Statistic::Frame::ID % 200 == 0) {
                cpu_max_time = cpu_max_time_200;
                cpu_max_time_200 = 0;
            }
            cpu_times[gpu_times_offset] = Statistic::Frame::CPUTime;
            cpu_times_offset = (cpu_times_offset + 1) % IM_ARRAYSIZE(cpu_times);
            sprintf(overlay, "CPU time: %d", Statistic::Frame::CPUTime);
            ImGui::PlotLines("", cpu_times, IM_ARRAYSIZE(cpu_times), cpu_times_offset, overlay, 0.0f, cpu_max_time, ImVec2(205, 50));

            if (Variables::ShowMemStat) {
                ImGui::Text("Total GPU memory %d", Statistic::GPUMemory::TotalMemory);
                //ImGui::ProgressBar(Statistic::GPUMemory::DedicatedMemory / static_cast<float>(Statistic::GPUMemory::TotalMemory), ImVec2(140.0f, 0.0f));
                //ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                //ImGui::Text("dedicated");
                ImGui::ProgressBar(Statistic::GPUMemory::AllocatedMemory / static_cast<float>(Statistic::GPUMemory::TotalMemory), ImVec2(140.0f, 0.0f));
                ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                ImGui::Text("allocated");
                ImGui::ProgressBar(Statistic::GPUMemory::FreeMemory / static_cast<float>(Statistic::GPUMemory::TotalMemory), ImVec2(140.0f, 0.0f));
                ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                ImGui::Text("free");
                ImGui::ProgressBar(Statistic::GPUMemory::EvictedMemory / static_cast<float>(Statistic::GPUMemory::TotalMemory), ImVec2(140.0f, 0.0f));
                ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                ImGui::Text("evicted");
            }
            ImGui::End();

            // Create zoom window
            ImGui::SetNextWindowPos(ImVec2(10, statisticWindowPos + 250), ImGuiCond_Once);
            ImGui::SetNextWindowSize(ImVec2(220, 55), ImGuiCond_Once);
            ImGui::Begin("Magnifier");
            ImGui::Checkbox("enabled", &Variables::Zoom::Enabled);
            ImGui::SameLine(90.0f, 0.0f); ImGui::SetNextItemWidth(90.0f);
            ImGui::SliderInt("zoom", &Variables::Zoom::Factor, 2, 10, "%d");
            ImGui::End();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            if (Variables::Debug) {
                glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
            }
        }
    }


    //-----------------------------------------------------------------------------
    // Name: WindowSizeChanged()
    // Desc: internal
    //-----------------------------------------------------------------------------
    void WindowSizeChanged(GLFWwindow* window, int width, int height) {
        height = glm::max(height, 1);

        glViewport(0, 0, width, height);
        glm::ivec2 newWindowSize = glm::ivec2(width, height);

        if (User::WindowSizeChanged)
            User::WindowSizeChanged(newWindowSize);

        Variables::WindowSize = newWindowSize;
    }


    //-----------------------------------------------------------------------------
    // Name: WindowClosed()
    // Desc: internal
    //-----------------------------------------------------------------------------
    void WindowClosed(GLFWwindow* window) {
        if (Callbacks::User::OpenGLRelease) {
            Callbacks::User::OpenGLRelease();
        }
    }


    //-----------------------------------------------------------------------------
    // Name: KeyboardChanged()
    // Desc: internal
    //-----------------------------------------------------------------------------
    void KeyboardChanged(GLFWwindow* window, int key, int scancode, int action, int mods) {
        if ((action != GLFW_PRESS) && (action != GLFW_REPEAT))
            return;

        switch(key) {
        case GLFW_KEY_Z: Variables::Shader::SceneZOffset += (mods == GLFW_MOD_SHIFT) ? -0.5f : 0.5f; return;
        case GLFW_KEY_I: Variables::Shader::Int += (mods == GLFW_MOD_SHIFT) ? -1 : 1; return;
        case GLFW_KEY_F: Variables::Shader::Float += (mods == GLFW_MOD_SHIFT) ? -0.01f : 0.01f; return;
        case GLFW_KEY_U: Variables::Shader::UserTest = !Variables::Shader::UserTest; return;
        case GLFW_KEY_C:
            fprintf(stderr, "\n");
            compileShaders();
            return;
        }

        if ((action == GLFW_PRESS) || (action == GLFW_REPEAT)) {
            if (key == GLFW_KEY_ESCAPE)
                Variables::AppClose = true;
      
            if (User::KeyboardChanged)
                User::KeyboardChanged(key, action, mods);
        }
    }


    //-----------------------------------------------------------------------------
    // Name: MouseButtonChanged()
    // Desc: internal
    //-----------------------------------------------------------------------------
    void MouseButtonChanged(GLFWwindow* window, int button, int action, int mods) {
        Variables::MouseLeftPressed  = ((button == GLFW_MOUSE_BUTTON_LEFT) && (action == GLFW_PRESS));
        Variables::MouseRightPressed = ((button == GLFW_MOUSE_BUTTON_RIGHT) && (action == GLFW_PRESS));
        if (Variables::MouseRightPressed)
            Variables::Zoom::Center = Variables::MousePosition;
    }


    //-----------------------------------------------------------------------------
    // Name: MousePositionChanged()
    // Desc: internal
    //-----------------------------------------------------------------------------
    void MousePositionChanged(GLFWwindow* window, double x, double y) {
        bool const leftMouse  = Variables::MouseLeftPressed  && !ImGui::GetIO().MouseDownOwned[0];
        bool const rightMouse = Variables::MouseRightPressed && !ImGui::GetIO().MouseDownOwned[1];
        glm::ivec2 lastMousePos  = Variables::MousePosition;
        Variables::MousePosition = glm::ivec2(static_cast<int>(x), static_cast<int>(y));

        if (leftMouse) {
            Variables::Shader::SceneRotation.x += 0.9f * static_cast<float>(y - lastMousePos.y);
            Variables::Shader::SceneRotation.y += 0.9f * static_cast<float>(x - lastMousePos.x);
        } 
        if (rightMouse) {
            Variables::Zoom::Center  = Variables::MousePosition;
        }
    }

    //-----------------------------------------------------------------------------
    // Name: PrintOGLDebugLog()
    // Desc: 
    //-----------------------------------------------------------------------------
    void
#ifdef _WIN32
    __stdcall
#endif
    PrintOGLDebugLog(GLenum source, GLenum type, GLuint id, GLenum severity,
                            GLsizei length, const GLchar* message, const GLvoid* userParam) {
        // Disable buffer info message on nVidia drivers
        if ((id == 0x20070) && (severity == GL_DEBUG_SEVERITY_LOW))
            return;

        switch(source) {
        case GL_DEBUG_SOURCE_API            : fprintf(stderr, "Source  : API\n"); break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM  : fprintf(stderr, "Source  : window system\n"); break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: fprintf(stderr, "Source  : shader compiler\n"); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY    : fprintf(stderr, "Source  : third party\n"); break;
        case GL_DEBUG_SOURCE_APPLICATION    : fprintf(stderr, "Source  : application\n"); break;
        case GL_DEBUG_SOURCE_OTHER          : fprintf(stderr, "Source  : other\n"); break;
        default                             : fprintf(stderr, "Source  : unknown\n"); break;
        }

        switch(type) {
        case GL_DEBUG_TYPE_ERROR              : fprintf(stderr, "Type    : error\n"); break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: fprintf(stderr, "Type    : deprecated behaviour\n"); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR : fprintf(stderr, "Type    : undefined behaviour\n"); break;
        case GL_DEBUG_TYPE_PORTABILITY        : fprintf(stderr, "Type    : portability issue\n"); break;
        case GL_DEBUG_TYPE_PERFORMANCE        : fprintf(stderr, "Type    : performance issue\n"); break;
        case GL_DEBUG_TYPE_OTHER              : fprintf(stderr, "Type    : other\n"); break;
        default                               : fprintf(stderr, "Type    : unknown\n"); break;
        }

        fprintf(stderr, "ID      : 0x%x\n", id);

        switch(severity) {
        case GL_DEBUG_SEVERITY_HIGH  : fprintf(stderr, "Severity: high\n"); break;
        case GL_DEBUG_SEVERITY_MEDIUM: fprintf(stderr, "Severity: medium\n"); break;
        case GL_DEBUG_SEVERITY_LOW   : fprintf(stderr, "Severity: low\n"); break;
        default                      : fprintf(stderr, "Severity: unknown\n"); break;
        }

        fprintf(stderr, "Message : %s\n", message);
        fprintf(stderr, "-------------------------------------------------------------------------------\n");
    }

    //-----------------------------------------------------------------------------
    // Name: glfwError()
    // Desc: 
    //-----------------------------------------------------------------------------
    void glfwError(int error, const char* description) {
        fprintf(stderr, "GLFW error: %s\n", description);
    }
}; // end of namespace Callbacks


//-----------------------------------------------------------------------------
// Name: ShowMagnifier()
// Desc: 
//-----------------------------------------------------------------------------
void ShowMagnifier() {
    if (!Variables::Zoom::Enabled)
        return;

    static GLuint               s_MagTexture              = 0;
    static GLsizei const        s_MagWindowResolution     = 200;
    static GLsizei const        s_MagWindowHalfResolution = s_MagWindowResolution >> 1;
    static GLsizei              s_MagTextureResolution    = 0;
    static std::vector<GLubyte> s_Pixels(s_MagWindowResolution * s_MagWindowResolution * 4);

    GLsizei const magTextureResolution     = s_MagWindowResolution / Variables::Zoom::Factor;
    GLsizei const magTextureHalfResolution = magTextureResolution >> 1;
    if (!s_MagTexture || (s_MagTextureResolution != magTextureResolution)) {
        glDeleteTextures(1, &s_MagTexture);
        s_MagTextureResolution = magTextureResolution;

        glCreateTextures(GL_TEXTURE_2D, 1, &s_MagTexture);
        glTextureParameteri(s_MagTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTextureParameteri(s_MagTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTextureParameteri(s_MagTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(s_MagTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameterfv(s_MagTexture, GL_TEXTURE_BORDER_COLOR, &glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)[0]);
        glTextureStorage2D(s_MagTexture, 1, GL_RGBA8, s_MagTextureResolution, s_MagTextureResolution);
    }
    
    GLint const invY = Variables::WindowSize.y - Variables::Zoom::Center.y;
    GLint const x = glm::clamp(Variables::Zoom::Center.x - magTextureHalfResolution, 0, Variables::WindowSize.x - magTextureResolution);
    GLint const y = glm::clamp(invY - magTextureHalfResolution, 0, Variables::WindowSize.y - magTextureResolution);
    glReadPixels(x, y, s_MagTextureResolution, s_MagTextureResolution, GL_RGBA, GL_UNSIGNED_BYTE, &s_Pixels[0]);
    glTextureSubImage2D(s_MagTexture, 0, 0, 0, s_MagTextureResolution, s_MagTextureResolution, GL_RGBA, GL_UNSIGNED_BYTE, &s_Pixels[0]);

    // Show magnificier texture
    GLint const x1 = glm::clamp(Variables::Zoom::Center.x - s_MagWindowHalfResolution, 0, Variables::WindowSize.x - s_MagWindowResolution);
    GLint const y1 = glm::clamp(invY - s_MagWindowHalfResolution, 0, Variables::WindowSize.y - s_MagWindowResolution);
    Tools::Texture::Show2DTexture(s_MagTexture, x1, y1, s_MagWindowResolution, s_MagWindowResolution, 0, true);
}

//-----------------------------------------------------------------------------
// Name: common_main()
// Desc: 
//-----------------------------------------------------------------------------
int common_main(int window_width, int window_height, const char* window_title,
                int* opengl_config,
                TInitGLCallback               cbUserInitGL,
                TReleaseOpenGLCallback        cbUserReleaseGL,
                TShowGUICallback              cbGUI,
                TDisplayCallback              cbUserDisplay,
                TWindowSizeChangedCallback    cbUserWindowSizeChanged,
                TKeyboardChangedCallback      cbUserKeyboardChanged,
                TMouseButtonChangedCallback   cbUserMouseButtonChanged,
                TMousePositionChangedCallback cbUserMousePositionChanged) {
    // Setup user callback functions
    assert(cbUserDisplay && cbUserInitGL);
    Callbacks::User::Display = cbUserDisplay;
    Callbacks::User::ShowGUI = cbGUI;
    Callbacks::User::OpenGLRelease = cbUserReleaseGL;
    Callbacks::User::WindowSizeChanged = cbUserWindowSizeChanged;
    Callbacks::User::KeyboardChanged = cbUserKeyboardChanged;
    Callbacks::User::MouseButtonChanged = cbUserMouseButtonChanged;
    Callbacks::User::MousePositionChanged = cbUserMousePositionChanged;

    // Setup internal variables
    Variables::WindowSize = glm::ivec2(window_width, window_height);

    // Setup temporary variables
    bool bDisableVSync     = false;
    bool bShowRotation     = false;
    bool bShowZOffset      = false;
    bool bAutoSwapDisabled = false;

    // Intialize GLFW
    glfwSetErrorCallback(Callbacks::glfwError);
    if (!glfwInit())
        return 1;

    int* pConfig = opengl_config;
    while (*pConfig != 0) {
        const int hint = *pConfig++;
        const int value = *pConfig++;

        switch (hint) {
        case PGR2_SHOW_MEMORY_STATISTICS:
            Variables::ShowMemStat = (value == GL_TRUE);
            break;
        case PGR2_DISABLE_VSYNC:
            bDisableVSync = (value == GL_TRUE);
            break;
        case PGR2_DISABLE_BUFFER_SWAP:
            bAutoSwapDisabled = (value == GL_TRUE);
            break;
        default:
            glfwWindowHint(hint, value);
            if (hint == GLFW_OPENGL_DEBUG_CONTEXT)
                Variables::Debug = (value == GL_TRUE);
            if ((hint == GLFW_CONTEXT_VERSION_MAJOR) && (value < 4)) {
                fprintf(stderr, "Error: OpenGL version must be 4 or higher\n");
                return 2;
            }
        }
    }

    // Create a window
    Variables::Window = glfwCreateWindow(Variables::WindowSize.x, Variables::WindowSize.y, window_title, nullptr, nullptr);
    if (!Variables::Window) {
        fprintf(stderr, "Error: unable to create window\n");
        return 3;
    }
    glfwSetWindowPos(Variables::Window, 100, 100);
    glfwMakeContextCurrent(Variables::Window);
    glfwSetInputMode(Variables::Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "GLEW error: %s\n", glewGetErrorString(err));
        return 4;
    }

    // Print debug info
    fprintf(stderr, "VENDOR  : %s\nVERSION : %s\nRENDERER: %s\nGLSL    : %s\n", glGetString(GL_VENDOR),
        glGetString(GL_VERSION),
        glGetString(GL_RENDERER),
        glGetString(GL_SHADING_LANGUAGE_VERSION));
    fprintf(stderr, "-------------------------------------------------------------------------------\n");
    
    // Init GUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(Variables::Window, true);
    ImGui_ImplOpenGL3_Init("#version 400");
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Enable OGL debug 
    if (Variables::Debug && glfwExtensionSupported("GL_ARB_debug_output")) {
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(Callbacks::PrintOGLDebugLog, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
    else
        Variables::Debug = false;

    // Disable VSync if required
    if (bDisableVSync) {
        glfwSwapInterval(0);
        fprintf(stderr, "VSync is disabled.\n");
    }
    else
        fprintf(stderr, "VSync is enabled!\n");

    // Check 
    if (Variables::ShowMemStat) {
        Variables::ShowMemStat = glfwExtensionSupported("GL_NVX_gpu_memory_info") == GLFW_TRUE;
        if (Variables::ShowMemStat) {
            glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &Statistic::GPUMemory::DedicatedMemory);
            glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &Statistic::GPUMemory::TotalMemory);
        }
    }

    // Init OGL
    if (cbUserInitGL) {
        cbUserInitGL();
        fprintf(stderr, "OpenGL initialized...\n");
        fprintf(stderr, "-------------------------------------------------------------------------------\n");
    }

    glGenQueries(OpenGL::NumQueries, OpenGL::Query);

    // Set GLFW event callbacks
    glfwSetWindowSizeCallback(Variables::Window, Callbacks::WindowSizeChanged);
    glfwSetWindowCloseCallback(Variables::Window, Callbacks::WindowClosed);
    glfwSetMouseButtonCallback(Variables::Window, Callbacks::MouseButtonChanged);
    glfwSetCursorPosCallback(Variables::Window, Callbacks::MousePositionChanged);
    glfwSetKeyCallback(Variables::Window, Callbacks::KeyboardChanged);

    // Main loop
    GLuint64 gpu_frame_start;
    GLuint64 gpu_frame_end;
    while (!glfwWindowShouldClose(Variables::Window) && !Variables::AppClose) {
        glfwPollEvents();

        // Increase frame counter
        Statistic::Frame::ID++;

        // Update transformations and default variables if used
        {
            Variables::Transform.ModelView           = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -Variables::Shader::SceneZOffset));
            Variables::Transform.ModelView           = glm::rotate(Variables::Transform.ModelView, Variables::Shader::SceneRotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
            Variables::Transform.ModelView           = glm::rotate(Variables::Transform.ModelView, Variables::Shader::SceneRotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
            Variables::Transform.Model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -Variables::Shader::SceneZOffset));
            Variables::Transform.ModelViewInverse    = glm::inverse(Variables::Transform.ModelView);
            Variables::Transform.Projection          = glm::perspective(60.0f, float(Variables::WindowSize.x) / Variables::WindowSize.y, 0.1f, 1000.0f);
            Variables::Transform.ModelViewProjection = Variables::Transform.Projection * Variables::Transform.ModelView;
            glGetFloatv(GL_VIEWPORT, &Variables::Transform.Viewport.x);

            for (size_t i = 0; i < OpenGL::programs.size(); i++) {
                const OpenGL::Program& program = OpenGL::programs[i];
                glUseProgram(program.id);
                if (program.MVPMatrix > -1)
                    glUniformMatrix4fv(program.MVPMatrix, 1, GL_FALSE, &Variables::Transform.ModelViewProjection[0][0]);
                if (program.ModelViewMatrix > -1)
                    glUniformMatrix4fv(program.ModelViewMatrix, 1, GL_FALSE, &Variables::Transform.ModelView[0][0]);
                if (program.ModelViewMatrixInv > -1)
                    glUniformMatrix4fv(program.ModelViewMatrix, 1, GL_FALSE, &Variables::Transform.ModelViewInverse[0][0]);
                if (program.ProjectionMatrix > -1)
                    glUniformMatrix4fv(program.ProjectionMatrix, 1, GL_FALSE, &Variables::Transform.Projection[0][0]);
                if (program.Viewport > -1)
                    glUniform4fv(program.Viewport, 1, &Variables::Transform.Viewport.x);
                if (program.ZOffset > -1)
                    glUniform1f(program.ZOffset, Variables::Shader::SceneZOffset);
                if (program.UserVariableInt > -1)
                    glUniform1i(program.UserVariableInt, Variables::Shader::Int);
                if (program.UserVariableFloat > -1)
                    glUniform1f(program.UserVariableFloat, Variables::Shader::Float);
                if (program.FrameCounter > -1)
                    glUniform1i(program.FrameCounter, Statistic::Frame::ID);
            }
        }
        glUseProgram(0);

        // Clean the pipeline
        // glFinish();

        const std::chrono::high_resolution_clock::time_point cpu_start = std::chrono::high_resolution_clock::now();
        glQueryCounter(OpenGL::Query[OpenGL::FrameStartQuery], GL_TIMESTAMP);
        if (Callbacks::User::Display)
            Callbacks::User::Display();
        glQueryCounter(OpenGL::Query[OpenGL::FrameEndQuery], GL_TIMESTAMP);
        const std::chrono::high_resolution_clock::time_point cpu_end = std::chrono::high_resolution_clock::now();
        Statistic::Frame::CPUTime = static_cast<int>(std::chrono::duration_cast<std::chrono::microseconds>(cpu_end - cpu_start).count());

        // Get memory statistics if required
        if (Variables::ShowMemStat) {
            glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &Statistic::GPUMemory::FreeMemory);
            glGetIntegerv(GL_GPU_MEMORY_INFO_EVICTED_MEMORY_NVX, &Statistic::GPUMemory::EvictedMemory);
            Statistic::GPUMemory::AllocatedMemory = Statistic::GPUMemory::TotalMemory - Statistic::GPUMemory::FreeMemory; 
        }

        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        glGetQueryObjectui64v(OpenGL::Query[OpenGL::FrameStartQuery], GL_QUERY_RESULT, &gpu_frame_start);
        glGetQueryObjectui64v(OpenGL::Query[OpenGL::FrameEndQuery], GL_QUERY_RESULT, &gpu_frame_end);
        Statistic::Frame::GPUTime = static_cast<int>(gpu_frame_end - gpu_frame_start);

        // Count FPS
        static unsigned long GPUTimeSum    = 0;
        static unsigned int  FPSFrameCount = 0;
        FPSFrameCount++;
        GPUTimeSum += Statistic::Frame::GPUTime;
        if (GPUTimeSum > 1000000) {
            Statistic::FPS = FPSFrameCount * 1000000000.0f / static_cast<float>(GPUTimeSum);
            GPUTimeSum     = 0;
            FPSFrameCount  = 0;
        }

        // Show Magnifier
        ShowMagnifier();

        // Render GUI
        Callbacks::GUI::Show(nullptr);

        // Present frame buffer
        if (!bAutoSwapDisabled)
            glfwSwapBuffers(Variables::Window);

        glfwPollEvents();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(Variables::Window);

    // Terminate GLFW
    glfwTerminate();

    return 0;
} 


//-----------------------------------------------------------------------------
// Name: common_main()
// Desc: 
//-----------------------------------------------------------------------------
int common_main(int window_width, int window_height, const char* window_title,
                TInitGLCallback               cbUserInitGL,
                TReleaseOpenGLCallback        cbUserReleaseGL,
                TShowGUICallback              cbGUI,
                TDisplayCallback              cbUserDisplay,
                TWindowSizeChangedCallback    cbUserWindowSizeChanged,
                TKeyboardChangedCallback      cbUserKeyboardChanged,
                TMouseButtonChangedCallback   cbUserMouseButtonChanged,
                TMousePositionChangedCallback cbUserMousePositionChanged) {
    return common_main(window_width, window_height, 
                       window_title,
                       nullptr,
                       cbUserInitGL,
                       cbUserReleaseGL,
                       cbGUI,
                       cbUserDisplay,
                       cbUserWindowSizeChanged,
                       cbUserKeyboardChanged,
                       cbUserMouseButtonChanged,
                       cbUserMousePositionChanged);
}
