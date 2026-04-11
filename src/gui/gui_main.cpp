#include "gui_main.h"
#include "../utils/path.h"
#include "history.h"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

const std::shared_ptr<spdlog::logger> gui_logger =
    spdlog::default_logger()->clone("\033[32mgui\033[0m");

namespace {
GLFWwindow *window = nullptr;
ImFont *font_sans;
void glfw_error_callback(int error, const char *description) {
    gui_logger->error("GLFW error {}: {}", error, description);
}
void draw() { draw_history(); }
} // namespace

auto get_font_sans() -> ImFont * { return font_sans; }

auto init_gui() -> int {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        gui_logger->error("Failed to initialize GLFW");
        return 1;
    }

    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(1280, 720, "EPSP", nullptr, nullptr);
    if (window == nullptr) {
        gui_logger->error("Failed to create GLFW window");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;

    std::filesystem::path font_sans_path =
        get_executable_dir() / "assets" / "NotoSans-Regular.ttf";

    font_sans = io.Fonts->AddFontFromFileTTF((font_sans_path.c_str()), 16.0F);

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    return 0;
}

void gui_loop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        draw();

        ImGui::Render();
        int32_t display_w = 0;
        int32_t display_h = 0;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.094F, 0.098F, 0.149F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
}

void cleanup_gui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}
