#pragma once

#include "Colors.h"

// A rectangular region of the window in pixels.
// (x, y) is the bottom-left corner, because OpenGL's origin is bottom-left.
struct Section {
    int x, y, width, height;
};

// The four sections the window is divided into
struct Layout {
    Section animation; // left half, top 70%     -> pendulum animation
    Section working;   // left half, bottom 30%  -> working shown while animation runs
    Section graph;     // right half, top 50%    -> reserved for matplotlib graph
    Section controls;  // right half, bottom 50% -> gravity, mass, string length inputs
};

// Split ratios (change these to resize the sections)
constexpr float LEFT_WIDTH_RATIO       = 0.50f; // left half vs right half
constexpr float ANIMATION_HEIGHT_RATIO = 0.70f; // animation vs working (left side)
constexpr float GRAPH_HEIGHT_RATIO     = 0.50f; // graph vs controls (right side)
constexpr int   SECTION_GAP            = 4;     // border thickness between sections (px)

Layout computeLayout(int windowWidth, int windowHeight);
void drawSection(const Section& section, const Color& color);
int Canvas();
