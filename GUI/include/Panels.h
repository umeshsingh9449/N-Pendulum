#pragma once

#include <imgui.h>
#include <include/Canvas.h>
#include <include/Pendulum.h>

// Fonts, colors and spacing for ImGui + ImPlot (dpiScale = monitor scale, e.g. 1.25 for 125%)
void setupStyle(float dpiScale);

// Opens a borderless, fixed ImGui window that exactly covers one section.
// Always call ImGui::End() afterwards, even if this returns false.
bool beginSectionWindow(const char* id, const Section& section, float displayHeight,
                        ImGuiWindowFlags extraFlags = 0);

// Colored heading at the top of a section
void sectionTitle(const char* text);

void drawWorkingPanel(const Simulation& sim, const Section& section, float displayHeight);
void drawGraphPanel(const Simulation& sim, const Section& section, float displayHeight);
void drawControlsPanel(Simulation& sim, const Section& section, float displayHeight);
