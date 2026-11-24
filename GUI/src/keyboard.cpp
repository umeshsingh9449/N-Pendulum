#include <include/keyboard.h>
#include <imgui.h>

void handleKeyboard(Simulation& sim){
    // Don't steal keys while the user is typing a value into a slider (Ctrl+Click)
    if(ImGui::GetIO().WantTextInput) return;

    if(ImGui::IsKeyPressed(ImGuiKey_Space, false)) sim.running = !sim.running;
    if(ImGui::IsKeyPressed(ImGuiKey_R, false))     sim.reset();
}
