#include "history.h"
#include "imgui.h"

void draw_history() {

    static float_t width = 300.0F;
    static bool open = true;

    width = open ? 300.0F : 0.0F;

    float_t offset_y = 50.0F;
    ImGuiIO &io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, offset_y));
    ImGui::SetNextWindowSize(
        ImVec2(width, static_cast<float>(io.DisplaySize.y - offset_y)));

    ImGui::Begin("History", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove);
    ImGui::End();
}
