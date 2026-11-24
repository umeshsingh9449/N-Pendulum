========================================================
Read me for Every function used in the entire codebase

1. glfw_error_callback(int error, const char* description)   [GUI/src/Canvas.cpp]
   Prints GLFW errors to the terminal. Registered with glfwSetErrorCallback.

2. inset(const Section& s, int pad)   [GUI/src/Canvas.cpp]
   Returns the section shrunk by 'pad' pixels on every side, which leaves the
   border gap between neighbouring sections.

3. computeLayout(int windowWidth, int windowHeight)   [GUI/src/Canvas.cpp]
   Splits the window into the 4 sections (animation, working, graph, controls)
   using the ratios in Canvas.h. Called every frame so the layout follows resizes.

4. drawSection(const Section& section, const Color& color)   [GUI/src/Canvas.cpp]
   Sets the OpenGL viewport + scissor to one section and clears it with its color.
   Anything drawn after it lands inside that section.

5. Canvas()   [GUI/src/Canvas.cpp]
   Creates the maximized window + OpenGL 3.3 context, runs the render loop that
   draws the 4 sections, and cleans up on close. Returns 0 on success, -1 on failure.

-------------------------- Physics (GUI/src/Pendulum.cpp) --------------------------

6. History::push / clear / size / fromEnd(k)
   Ring buffer of Samples (t, θ, ω, energies, bob x/y) recorded every frame.
   fromEnd(0) is the newest sample. Feeds the live graph and the bob trail.

7. SinglePendulum::angularAcceleration(theta, omega)
   α = -(g/L)·sin θ - b·ω   (equation of motion).

8. SinglePendulum::step(dt)
   Advances θ, ω and time by dt using Runge-Kutta 4.

9. SinglePendulum::kineticEnergy / potentialEnergy / tension / smallAnglePeriod
   ½·m·L²·ω²,  m·g·L·(1 - cos θ),  m·(g·cos θ + L·ω²),  2π·√(L/g).

10. Simulation::reset()
    Puts the pendulum back to θ0, ω0, t = 0 and clears the history.

11. Simulation::update(frameDt)
    Runs fixed PHYSICS_DT (1 ms) steps for this frame's time (× speed), then record().

12. Simulation::record()
    Pushes the current state into the history.

-------------------------- Animation (GUI/src/Drawing.cpp) --------------------------

13. computeAnimationView(rectMin, rectMax, pendulum)
    Pivot position, pixels-per-meter scale and bob radius for the animation section.

14. AnimationView::toScreen(x, y)
    Meters (y up, relative to pivot) -> screen pixels (y down).

15. drawCeiling / drawReferenceLine / drawTrail / drawAngleArc / drawScaleBar
    Pieces of the animation: hatched ceiling, θ = 0 dashed line,
    fading bob trail, θ arc + label, "1 m" scale bar.

16. drawAnimationPanel(sim, section, displayHeight)
    Draws the whole left-top section (calls all of the above + handleBobDrag).

-------------------------- Input (GUI/src/Mouse.cpp, GUI/src/keyboard.cpp) --------------------------

17. handleBobDrag(sim, view)
    Click and drag the bob to set the start angle; returns true while hovered/dragged.

18. isDraggingBob()
    True while the bob is held with the mouse.

19. handleKeyboard(sim)
    Space = play/pause, R = reset (ignored while typing into a slider).

-------------------------- Panels (GUI/src/Panels.cpp) --------------------------

20. setupStyle(dpiScale)
    Loads fonts (Segoe UI + Consolas) and sets ImGui/ImPlot colors, scaled for the monitor DPI.

21. beginSectionWindow(id, section, displayHeight, extraFlags)
    Fixed, borderless, transparent ImGui window covering one section. Always pair with ImGui::End().

22. sectionTitle(text)
    Colored heading at the top of a section.

23. drawWorkingPanel(sim, section, displayHeight)
    Left-bottom: equation with live numbers substituted, state values, energies.

24. drawGraphPanel(sim, section, displayHeight)
    Right-top: live ImPlot graphs - θ & ω vs time, energy vs time, phase space.

25. drawControlsPanel(sim, section, displayHeight)
    Right-bottom: play/pause/reset, L, m, θ0, ω0, g (+ presets), damping, speed, graph window.

26. sliderDouble(label, value, min, max, format)
    ImGui slider for a double (ImGui's built-in ones are float/int).

========================================================
Build & run (from the GUI folder)

  g++ src/Canvas.cpp src/Pendulum.cpp src/Drawing.cpp src/Panels.cpp src/Mouse.cpp src/keyboard.cpp -I. -Iexternal/imgui -Iexternal/imgui/backends -Iexternal/implot -Lexternal/lib -limgui -lglad -lglfw3 -lopengl32 -lgdi32 -o src/Canvas.exe
  ./src/Canvas.exe

external/lib/libimgui.a = Dear ImGui v1.93.0 + ImPlot v1.1, prebuilt once (sources in external/).
