#include <include/Drawing.h>
#include <include/Mouse.h>
#include <include/Panels.h>
#include <cmath>
#include <cstdio>
#include <algorithm>

// Colors used in the animation (IM_COL32 = r, g, b, a from 0 to 255)
static const ImU32 COL_ROD       = IM_COL32(220, 225, 235, 255);
static const ImU32 COL_PIVOT     = IM_COL32(240, 240, 240, 255);
static const ImU32 COL_CEILING   = IM_COL32(150, 160, 175, 255);
static const ImU32 COL_REFERENCE = IM_COL32(255, 255, 255,  60);
static const ImU32 COL_ARC       = IM_COL32(255, 200,  80, 230);
static const ImU32 COL_BOB       = IM_COL32(235,  90,  70, 255);
static const ImU32 COL_BOB_SHINE = IM_COL32(255, 165, 145, 255);
static const ImU32 COL_HIGHLIGHT = IM_COL32(255, 255, 255, 200);
static const ImU32 COL_TEXT_DIM  = IM_COL32(200, 205, 215, 150);

constexpr int   TRAIL_SAMPLES   = 90;   // how many recent frames the bob trail shows (~1.5 s)
constexpr float VIEW_MIN_LENGTH = 1.5f; // strings up to this length (m) share one scale, so changing L is visible

ImVec2 AnimationView::toScreen(double x, double y) const {
    return ImVec2(pivot.x + (float)x * pixelsPerMeter,
                  pivot.y - (float)y * pixelsPerMeter);
}

AnimationView computeAnimationView(ImVec2 rectMin, ImVec2 rectMax, const SinglePendulum& p){
    float w = rectMax.x - rectMin.x;
    float h = rectMax.y - rectMin.y;

    AnimationView view;
    view.rectMin = rectMin;
    view.rectMax = rectMax;

    // Pivot in the middle so the bob also fits when it swings over the top
    view.pivot = ImVec2(rectMin.x + w * 0.5f, rectMin.y + h * 0.5f);

    // Longer strings zoom the view out so the whole swing circle still fits
    float reach = 0.44f * std::min(w, h);
    view.pixelsPerMeter = reach / std::max((float)p.length, VIEW_MIN_LENGTH);

    // Bob size grows with mass (volume ~ mass -> radius ~ cube root of mass)
    float dpi = ImGui::GetStyle().FontScaleDpi;
    view.bobRadius = std::clamp(11.0f * (float)std::cbrt(p.mass), 6.0f, 30.0f) * dpi;
    return view;
}

// Hatched ceiling bar the pendulum hangs from
static void drawCeiling(ImDrawList* dl, const AnimationView& view, float dpi){
    float halfWidth = 45.0f * dpi;
    float hatch     = 8.0f * dpi;
    ImVec2 left (view.pivot.x - halfWidth, view.pivot.y);
    ImVec2 right(view.pivot.x + halfWidth, view.pivot.y);

    dl->AddLine(left, right, COL_CEILING, 2.0f * dpi);
    for(float x = left.x; x < right.x; x += hatch){
        dl->AddLine(ImVec2(x, view.pivot.y), ImVec2(x + hatch, view.pivot.y - hatch), COL_CEILING, 1.0f);
    }
}

// Dashed vertical line straight down from the pivot: the θ = 0 position
static void drawReferenceLine(ImDrawList* dl, const AnimationView& view, double length, float dpi){
    float total = (float)length * view.pixelsPerMeter;
    float dash  = 6.0f * dpi;
    for(float d = 0; d < total; d += 2 * dash){
        dl->AddLine(ImVec2(view.pivot.x, view.pivot.y + d),
                    ImVec2(view.pivot.x, view.pivot.y + std::min(d + dash, total)),
                    COL_REFERENCE, 1.0f * dpi);
    }
}

// Fading path of the bob over the last TRAIL_SAMPLES frames
static void drawTrail(ImDrawList* dl, const AnimationView& view, const History& history, float dpi){
    int n = std::min(TRAIL_SAMPLES, history.size());
    for(int k = 1; k < n; k++){
        const Sample& a = history.fromEnd(k - 1);
        const Sample& b = history.fromEnd(k);
        int alpha = (int)(180.0f * (1.0f - (float)k / n));
        dl->AddLine(view.toScreen(a.x, a.y), view.toScreen(b.x, b.y),
                    IM_COL32(235, 90, 70, alpha), 2.0f * dpi);
    }
}

// Arc from the vertical to the string, labelled with the current angle
static void drawAngleArc(ImDrawList* dl, const AnimationView& view, const SinglePendulum& p, float dpi){
    double thetaWrapped = std::remainder(p.theta, 2.0 * PI); // -π..π, so the arc never winds around
    if(std::abs(thetaWrapped) < 0.01) return;

    // ImGui angles: 0 = right, increasing clockwise on screen. Straight down = π/2.
    float down     = (float)(PI / 2.0);
    float bobAngle = down - (float)thetaWrapped;
    float radius   = std::min(0.3f * (float)p.length * view.pixelsPerMeter, 60.0f * dpi);

    dl->PathArcTo(view.pivot, radius, down, bobAngle, 32);
    dl->PathStroke(COL_ARC, ImDrawFlags_None, 2.0f * dpi);

    // Label on the other side of the vertical line, so it never sits on top of the string
    char label[32];
    snprintf(label, sizeof(label), "θ = %.1f°", thetaWrapped * 180.0 / PI);
    ImVec2 size = ImGui::CalcTextSize(label);
    float gap = 8.0f * dpi;
    float x = thetaWrapped > 0 ? view.pivot.x - gap - size.x : view.pivot.x + gap;
    dl->AddText(ImVec2(x, view.pivot.y + radius - size.y), COL_ARC, label);
}

// "1 m" scale bar in the bottom-right corner
static void drawScaleBar(ImDrawList* dl, const AnimationView& view, float dpi){
    float pad = 16.0f * dpi;
    float len = view.pixelsPerMeter;
    ImVec2 right(view.rectMax.x - pad, view.rectMax.y - pad);
    ImVec2 left (right.x - len, right.y);
    float tick = 5.0f * dpi;

    dl->AddLine(left, right, COL_TEXT_DIM, 1.5f * dpi);
    dl->AddLine(ImVec2(left.x,  left.y  - tick), ImVec2(left.x,  left.y  + tick), COL_TEXT_DIM, 1.5f * dpi);
    dl->AddLine(ImVec2(right.x, right.y - tick), ImVec2(right.x, right.y + tick), COL_TEXT_DIM, 1.5f * dpi);

    ImVec2 size = ImGui::CalcTextSize("1 m");
    dl->AddText(ImVec2((left.x + right.x - size.x) * 0.5f, left.y - tick - size.y), COL_TEXT_DIM, "1 m");
}

void drawAnimationPanel(Simulation& sim, const Section& section, float displayHeight){
    // NoInputs: clicks pass through to our own bob-dragging code instead of ImGui
    if(beginSectionWindow("##animation", section, displayHeight,
                          ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoScrollbar)){
        ImVec2 rectMin = ImGui::GetWindowPos();
        ImVec2 rectMax(rectMin.x + ImGui::GetWindowWidth(), rectMin.y + ImGui::GetWindowHeight());
        float dpi = ImGui::GetStyle().FontScaleDpi;

        AnimationView view = computeAnimationView(rectMin, rectMax, sim.pendulum);

        // Mouse first: dragging moves the bob, so everything below draws the new position
        bool highlightBob = handleBobDrag(sim, view);

        const SinglePendulum& p = sim.pendulum;
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 bob = view.toScreen(p.length * std::sin(p.theta), -p.length * std::cos(p.theta));

        drawCeiling(dl, view, dpi);
        drawReferenceLine(dl, view, p.length, dpi);
        drawTrail(dl, view, sim.history, dpi);
        drawAngleArc(dl, view, p, dpi);

        // String + pivot
        dl->AddLine(view.pivot, bob, COL_ROD, 2.5f * dpi);
        dl->AddCircleFilled(view.pivot, 4.5f * dpi, COL_PIVOT);

        // Bob with a small shine so it reads as a ball
        float r = view.bobRadius;
        dl->AddCircleFilled(bob, r, COL_BOB);
        dl->AddCircleFilled(ImVec2(bob.x - r * 0.3f, bob.y - r * 0.3f), r * 0.35f, COL_BOB_SHINE);
        if(highlightBob) dl->AddCircle(bob, r + 3.0f * dpi, COL_HIGHLIGHT, 0, 2.0f * dpi);

        drawScaleBar(dl, view, dpi);

        // Text overlay (top-left: title + status, bottom-left: hints)
        sectionTitle("ANIMATION");
        const char* status = isDraggingBob() ? "DRAGGING" : (sim.running ? "RUNNING" : "PAUSED");
        ImGui::TextDisabled("Single pendulum   t = %.2f s   %s", p.time, status);

        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ImGui::GetStyle().WindowPadding.y - ImGui::GetTextLineHeight());
        ImGui::TextDisabled("Drag the bob to set the start angle   |   Space: play/pause   |   R: reset");
    }
    ImGui::End();
}
