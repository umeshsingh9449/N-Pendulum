#include <include/Mouse.h>
#include <cmath>

static bool dragging   = false;
static bool wasRunning = false; // play state to restore when the bob is released

bool isDraggingBob(){
    return dragging;
}

bool handleBobDrag(Simulation& sim, const AnimationView& view){
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mouse = io.MousePos;

    const SinglePendulum& p = sim.pendulum;
    ImVec2 bob = view.toScreen(p.length * std::sin(p.theta), -p.length * std::cos(p.theta));

    float dx = mouse.x - bob.x;
    float dy = mouse.y - bob.y;
    float grabRadius = view.bobRadius + 6.0f; // a little bigger than the bob so it's easy to grab
    bool overBob = !io.WantCaptureMouse && (dx * dx + dy * dy <= grabRadius * grabRadius);

    if(!dragging && overBob){
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)){
            dragging   = true;
            wasRunning = sim.running;
            sim.running = false;
        }
    }

    if(dragging){
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);

        // Angle of the mouse around the pivot, measured from the downward vertical
        // (screen y points down, so atan2(dx, dy) is 0 straight below the pivot)
        double theta = std::atan2(mouse.x - view.pivot.x, mouse.y - view.pivot.y);
        sim.initialAngleDeg = theta * 180.0 / PI;
        sim.initialOmega    = 0.0; // released from rest
        sim.reset();

        if(!ImGui::IsMouseDown(ImGuiMouseButton_Left)){
            dragging    = false;
            sim.running = wasRunning;
        }
    }

    return overBob || dragging;
}
