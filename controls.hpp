//-----------------------------------------------------------------------------
//  [PGR2] Simple Shadow Mapping
//  23/04/2017
//-----------------------------------------------------------------------------
const char* help_message =
"[PGR2] Simple Shadow Mapping\n\
-------------------------------------------------------------------------------\n\
CONTROLS:\n\
   [i/I]   ... inc/dec value of user integer variable\n\
   [f/F]   ... inc/dec value of user floating-point variable\n\
   [z/Z]   ... move scene along z-axis\n\
   [s]     ... show/hide first depth map texture\n\
   [d/D]   ... change depth map resolution\n\
   [c]     ... compile shaders\n\
   [mouse] ... scene rotation (left button)\n\
-------------------------------------------------------------------------------";

// IMPLEMENTATION______________________________________________________________

//-----------------------------------------------------------------------------
// Name: updateLightViewMatrix()
// Desc: 
//-----------------------------------------------------------------------------
void updateLightViewMatrix() {
    // Compute light view matrix
    g_LightViewMatrix = glm::lookAt(g_LightPosition, 
                                    glm::vec3(0.0f, 0.0f, 0.0f),
                                    glm::normalize(glm::vec3(-g_LightPosition.y, g_LightPosition.x, 0.0f)));
}


//-----------------------------------------------------------------------------
// Name: compileShaders()
// Desc: 
//-----------------------------------------------------------------------------
void compileShaders(void *clientData) {
    // Create shader program object

    Tools::Shader::CreateShaderProgramFromFile(g_ProgramId[DepthTextureGeneration],
        "shadow_mapping_1st_pass.vs", nullptr, nullptr, nullptr, "shadow_mapping_1st_pass.fs");
    Tools::Shader::CreateShaderProgramFromFile(g_ProgramId[ShadowTest], "shadow_mapping_2nd_pass.vs",
        nullptr, nullptr, nullptr, "shadow_mapping_2nd_pass.fs");

    Tools::Shader::CreateShaderProgramFromFile(g_ProgramId[ShadowTestAliasFree],
        "3rd_pass_shadow_test.vs", nullptr, nullptr, "3rd_pass_shadow_test.gs", "3rd_pass_shadow_test.fs");
    Tools::Shader::CreateShaderProgramFromFile(g_ProgramId[RenderScene], "4th_pass_render_scene.vs",
        nullptr, nullptr, nullptr, "4th_pass_render_scene.fs");
    Tools::Shader::CreateShaderProgramFromFile(g_ProgramId[VisibilityMapGeneration], "1st_pass_visibility_map_generation.vs",
        nullptr, nullptr, nullptr, "1st_pass_visibility_map_generation.fs");
    Tools::Shader::CreateShaderProgramFromFile(g_ProgramId[ListBufferGeneration], "2nd_pass_list_buffer_generation.vs",
        nullptr, nullptr, nullptr, "2nd_pass_list_buffer_generation.fs");
}


//-----------------------------------------------------------------------------
// Name: showGUI()
// Desc: 
//-----------------------------------------------------------------------------
void showGUI(void* user) {
    ImGui::Begin("Controls");
    ImGui::SetWindowSize(ImVec2(220, 470), ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Render", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("show textures", &g_ShowDepthTexture);
    }

    if (ImGui::CollapsingHeader("Virtual Framebuffer", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SetNextItemWidth(120);
        int resolution = int(glm::round(glm::log2(g_Resolution / 128.0f)));
        if (ImGui::Combo("resolution", &resolution, " 128x128 \0 256x256\0 512x512 \0 1024x1024\0 2048x2048\0")) {
            g_Resolution = 128 << resolution;
        }
    }

    if (ImGui::CollapsingHeader("Shadow maps", ImGuiTreeNodeFlags_DefaultOpen))
    {
        int algorithm = g_ShadowMapsAlgo;
        ImGui::SetNextItemWidth(120);
        if (ImGui::Combo("Algorithm", &algorithm, " Depth Map\0 Alias-Free\0"))
        {
            if (g_ShadowMapsAlgo != algorithm) g_Switch = true;
            g_ShadowMapsAlgo = algorithm;
        }
    }

    if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SetNextItemWidth(190);
        ImGui::SliderFloat("x", &g_LightPosition.x, -30.0f, 30.0f, "%.3f");
        ImGui::SetNextItemWidth(190);
        ImGui::SliderFloat("y", &g_LightPosition.y, -30.0f, 30.0f, "%.3f");
        ImGui::SetNextItemWidth(190);
        ImGui::SliderFloat("z", &g_LightPosition.z, -30.0f, 30.0f, "%.3f");
    }

    ImGui::End();
    *static_cast<int *>(user) = 485;
}


//-----------------------------------------------------------------------------
// Name: keyboardChanged()
// Desc:
//-----------------------------------------------------------------------------
void keyboardChanged(int key, int action, int mods) {
    switch (key) {
    case GLFW_KEY_S: g_ShowDepthTexture = !g_ShowDepthTexture; break;
    case GLFW_KEY_D: g_Resolution       = (mods == GLFW_MOD_SHIFT) ? ((g_Resolution == 32) ? 2048 : (g_Resolution >> 1)) : ((g_Resolution == 2048) ? 32 : (g_Resolution << 1));
    }
}


//-----------------------------------------------------------------------------
// Name: main()
// Desc: 
//-----------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    int OGL_CONFIGURATION[] = {
        GLFW_CONTEXT_VERSION_MAJOR,  4,
        GLFW_CONTEXT_VERSION_MINOR,  0,
        GLFW_OPENGL_FORWARD_COMPAT,  GL_FALSE,
        GLFW_OPENGL_DEBUG_CONTEXT,   GL_TRUE,
        GLFW_OPENGL_PROFILE,         GLFW_OPENGL_COMPAT_PROFILE, // GLFW_OPENGL_CORE_PROFILE
        PGR2_SHOW_MEMORY_STATISTICS, GL_TRUE, 
        0
    };

    printf("%s\n", help_message);

    return common_main(1200, 900, "[PGR2] Alias Free Shadow Maps",
                       OGL_CONFIGURATION, // OGL configuration hints
                       initGL,            // Init GL callback function
                       nullptr,           // Release GL callback function
                       showGUI,           // Show GUI callback function
                       display,           // Display callback function
                       resizeWindow,      // Window resize callback function
                       keyboardChanged,   // Keyboard callback function
                       nullptr,           // Mouse button callback function
                       nullptr);          // Mouse motion callback function
}
