#pragma once

#include <imgui.h>
#include <include/Canvas.h>
#include <include/Pendulum.h>

// Where and how big the pendulum is drawn inside the animation section (screen pixels)
struct AnimationView {
    ImVec2 rectMin, rectMax; // animation section rectangle (top-left, bottom-right)
    ImVec2 pivot;            // screen position of the pivot
    float  pixelsPerMeter;
    float  bobRadius;

    ImVec2 toScreen(double x, double y) const; // meters (y up) -> pixels (y down)
};

AnimationView computeAnimationView(ImVec2 rectMin, ImVec2 rectMax, const SinglePendulum& p);
void drawAnimationPanel(Simulation& sim, const Section& section, float displayHeight);
