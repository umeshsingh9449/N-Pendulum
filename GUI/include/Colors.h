#pragma once

// Simple RGBA color (each channel from 0.0 to 1.0, same as glClearColor expects)
struct Color {
    float r, g, b, a;
};

// Background colors for every section of the window
namespace Colors {
    constexpr Color Border    = {0.05f, 0.05f, 0.07f, 1.0f}; // gap lines between sections
    constexpr Color Animation = {0.10f, 0.20f, 0.25f, 1.0f}; // left-top: pendulum animation
    constexpr Color Working   = {0.13f, 0.16f, 0.20f, 1.0f}; // left-bottom: live working/values
    constexpr Color Graph     = {0.18f, 0.18f, 0.22f, 1.0f}; // right-top: matplotlib graph
    constexpr Color Controls  = {0.20f, 0.25f, 0.25f, 1.0f}; // right-bottom: inputs (g, mass, length)
}
