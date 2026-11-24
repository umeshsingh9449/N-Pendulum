#include <include/Panels.h>
#include <implot.h>
#include <cmath>
#include <cstdio>
#include <algorithm>

static ImFont* monoFont = nullptr; // monospace font for numbers, so they don't jitter while changing

// Plot line colors (shared by the graph tabs)
static const ImVec4 COL_THETA     = ImVec4(1.00f, 0.78f, 0.31f, 1.0f); // same yellow as the angle arc
static const ImVec4 COL_OMEGA     = ImVec4(0.35f, 0.78f, 1.00f, 1.0f); // blue for ω
static const ImVec4 COL_KINETIC   = ImVec4(0.35f, 0.78f, 1.00f, 1.0f);
static const ImVec4 COL_POTENTIAL = ImVec4(0.92f, 0.35f, 0.27f, 1.0f);
static const ImVec4 COL_TOTAL     = ImVec4(0.85f, 0.85f, 0.90f, 1.0f);

static bool fileExists(const char* path){
    FILE* f = std::fopen(path, "rb");
    if(!f) return false;
    std::fclose(f);
    return true;
}

void setupStyle(float dpiScale){
    ImGuiIO& io = ImGui::GetIO();

    // Segoe UI for the interface, Consolas for numbers. Both ship with Windows and contain
    // the Greek letters we use (θ, ω, α, π); otherwise fall back to ImGui's built-in font.
    const char* uiFontPath   = "C:/Windows/Fonts/segoeui.ttf";
    const char* monoFontPath = "C:/Windows/Fonts/consola.ttf";

    ImFont* uiFont = fileExists(uiFontPath) ? io.Fonts->AddFontFromFileTTF(uiFontPath, 15.0f)
                                            : io.Fonts->AddFontDefault();
    monoFont = fileExists(monoFontPath) ? io.Fonts->AddFontFromFileTTF(monoFontPath, 14.0f) : uiFont;

    // ---- ImGui ----
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding    = ImVec2(14, 10);
    style.FramePadding     = ImVec2(8, 4);
    style.ItemSpacing      = ImVec2(10, 7);
    style.FrameRounding    = 4.0f;
    style.GrabRounding     = 4.0f;
    style.TabRounding      = 4.0f;
    style.WindowBorderSize = 0.0f;
    style.ScaleAllSizes(dpiScale);
    style.FontScaleDpi = dpiScale;

    ImVec4* c = style.Colors;
    c[ImGuiCol_FrameBg]          = ImVec4(0.09f, 0.12f, 0.14f, 1.0f);
    c[ImGuiCol_FrameBgHovered]   = ImVec4(0.14f, 0.20f, 0.23f, 1.0f);
    c[ImGuiCol_FrameBgActive]    = ImVec4(0.17f, 0.25f, 0.28f, 1.0f);
    c[ImGuiCol_SliderGrab]       = ImVec4(0.30f, 0.70f, 0.75f, 1.0f);
    c[ImGuiCol_SliderGrabActive] = ImVec4(0.45f, 0.85f, 0.90f, 1.0f);
    c[ImGuiCol_Button]           = ImVec4(0.17f, 0.38f, 0.42f, 1.0f);
    c[ImGuiCol_ButtonHovered]    = ImVec4(0.23f, 0.50f, 0.55f, 1.0f);
    c[ImGuiCol_ButtonActive]     = ImVec4(0.30f, 0.62f, 0.68f, 1.0f);
    c[ImGuiCol_Tab]              = ImVec4(0.12f, 0.16f, 0.19f, 1.0f);
    c[ImGuiCol_TabHovered]       = ImVec4(0.23f, 0.50f, 0.55f, 1.0f);
    c[ImGuiCol_TabSelected]      = ImVec4(0.17f, 0.38f, 0.42f, 1.0f);
    c[ImGuiCol_TableHeaderBg]    = ImVec4(0.12f, 0.16f, 0.19f, 1.0f);
    c[ImGuiCol_PlotHistogram]    = COL_KINETIC; // progress bar fill (energy split)

    // ---- ImPlot ----
    ImPlot::StyleColorsDark();
    ImPlotStyle& plotStyle = ImPlot::GetStyle();
    plotStyle.Colors[ImPlotCol_FrameBg] = ImVec4(0, 0, 0, 0);
    plotStyle.Colors[ImPlotCol_PlotBg]  = ImVec4(0.07f, 0.07f, 0.09f, 0.6f);
}

bool beginSectionWindow(const char* id, const Section& section, float displayHeight,
                        ImGuiWindowFlags extraFlags){
    // Sections are in OpenGL coordinates (origin bottom-left), ImGui uses top-left, so flip y
    ImGui::SetNextWindowPos(ImVec2((float)section.x, displayHeight - (float)(section.y + section.height)));
    ImGui::SetNextWindowSize(ImVec2((float)section.width, (float)section.height));

    // No title bar / moving / resizing, and transparent so the section color shows through
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
                           | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse
                           | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground
                           | ImGuiWindowFlags_NoBringToFrontOnFocus | extraFlags;
    return ImGui::Begin(id, nullptr, flags);
}

void sectionTitle(const char* text){
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.45f, 0.80f, 0.85f, 1.0f));
    ImGui::TextUnformatted(text);
    ImGui::PopStyleColor();
}

// ---------------------------------------------------------------------------
// Left-bottom: the working (equation with the current numbers substituted in)
// ---------------------------------------------------------------------------
void drawWorkingPanel(const Simulation& sim, const Section& section, float displayHeight){
    if(beginSectionWindow("##working", section, displayHeight)){
        sectionTitle("WORKING");
        ImGui::Separator();

        const SinglePendulum& p = sim.pendulum;
        double thetaWrapped = std::remainder(p.theta, 2.0 * PI); // -π..π for display
        double thetaDeg     = thetaWrapped * 180.0 / PI;
        double alpha        = p.angularAcceleration(p.theta, p.omega);
        double ke           = p.kineticEnergy();
        double pe           = p.potentialEnergy();
        double energy       = ke + pe;
        double drift        = energy - sim.initialEnergy;

        // Monospace + tighter rows so all lines fit in the 30% height
        ImGui::PushFont(monoFont, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 2.0f));
        if(ImGui::BeginTable("working", 3, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp)){
            ImGui::TableSetupColumn("Equation of motion", ImGuiTableColumnFlags_None, 1.1f);
            ImGui::TableSetupColumn("State",              ImGuiTableColumnFlags_None, 1.0f);
            ImGui::TableSetupColumn("Energy",             ImGuiTableColumnFlags_None, 1.1f);
            ImGui::TableHeadersRow();
            ImGui::TableNextRow();

            // ---- Equation of motion ----
            ImGui::TableNextColumn();
            ImGui::TextDisabled("α = -(g/L)·sin θ - b·ω");
            ImGui::Text("  = -(%.2f/%.2f)·sin(%.1f°)", p.g, p.length, thetaDeg);
            ImGui::Text("    - %.2f·(%.3f)", p.damping, p.omega);
            ImGui::Text("  = %.3f rad/s²", alpha);
            ImGui::Spacing();
            ImGui::TextDisabled("Small-angle period");
            ImGui::Text("T = 2π·sqrt(L/g)");
            ImGui::Text("  = %.3f s", p.smallAnglePeriod());

            // ---- State ----
            ImGui::TableNextColumn();
            ImGui::Text("t = %.3f s", p.time);
            ImGui::Text("θ = %.2f° (%.3f rad)", thetaDeg, thetaWrapped);
            ImGui::Text("ω = %.3f rad/s", p.omega);
            ImGui::Text("α = %.3f rad/s²", alpha);
            ImGui::Text("v = L·ω = %.3f m/s", p.length * p.omega);
            ImGui::Text("x = L·sin θ = %.3f m", p.length * std::sin(p.theta));
            ImGui::Text("y = -L·cos θ = %.3f m", -p.length * std::cos(p.theta));
            ImGui::Text("Tension = %.2f N", p.tension());

            // ---- Energy ----
            ImGui::TableNextColumn();
            ImGui::TextDisabled("KE = ½·m·L²·ω²");
            ImGui::Text("   = %.3f J", ke);
            ImGui::TextDisabled("PE = m·g·L·(1 - cos θ)");
            ImGui::Text("   = %.3f J", pe);
            ImGui::Text("E  = KE + PE = %.3f J", energy);
            ImGui::Text("ΔE = %+.4f J", drift); // damping loss (or numerical error when b = 0)

            // How the total energy is split between kinetic and potential right now
            float keShare = energy > 1e-12 ? (float)(ke / energy) : 0.0f;
            char split[48];
            snprintf(split, sizeof(split), "KE %.0f%%  |  PE %.0f%%", keShare * 100.0f, (1.0f - keShare) * 100.0f);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, COL_POTENTIAL);
            ImGui::ProgressBar(keShare, ImVec2(-1, 0), split);
            ImGui::PopStyleColor();

            ImGui::EndTable();
        }
        ImGui::PopStyleVar();
        ImGui::PopFont();
    }
    ImGui::End();
}

// ---------------------------------------------------------------------------
// Right-top: live graphs drawn straight from the recorded history
// ---------------------------------------------------------------------------
void drawGraphPanel(const Simulation& sim, const Section& section, float displayHeight){
    if(beginSectionWindow("##graph", section, displayHeight, ImGuiWindowFlags_NoScrollbar)){
        sectionTitle("LIVE GRAPH");

        const History& history = sim.history;
        int n = history.size();

        if(n > 0 && ImGui::BeginTabBar("graphTabs")){
            const Sample* d = history.data.data();

            // The history is a ring buffer of Sample structs: Offset tells ImPlot where the
            // oldest sample is, Stride tells it how far apart one field is between samples
            ImPlotSpec line;
            line.Offset     = history.offset;
            line.Stride     = sizeof(Sample);
            line.LineWeight = 2.0f;

            // Scrolling time axis: always show the last 'graphWindow' seconds
            double tMax = std::max(sim.pendulum.time, (double)sim.graphWindow);
            double tMin = tMax - sim.graphWindow;

            // Y axes fit to the data that is currently visible
            ImPlotAxisFlags fitY = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
            ImVec2 fill(-1, -1);

            // Legend in one row above the plot, so it never covers the lines
            ImPlotLegendFlags legendFlags = ImPlotLegendFlags_Outside | ImPlotLegendFlags_Horizontal;

            if(ImGui::BeginTabItem("Angle & angular velocity")){
                if(ImPlot::BeginPlot("##thetaOmega", fill)){
                    ImPlot::SetupAxes("time (s)", "θ (deg)", ImPlotAxisFlags_None, fitY);
                    ImPlot::SetupAxis(ImAxis_Y2, "ω (rad/s)", ImPlotAxisFlags_AuxDefault | fitY);
                    ImPlot::SetupAxisLimits(ImAxis_X1, tMin, tMax, ImPlotCond_Always);
                    ImPlot::SetupLegend(ImPlotLocation_North, legendFlags);

                    line.LineColor = COL_THETA;
                    ImPlot::PlotLine("θ (deg)", &d->t, &d->theta, n, line);

                    ImPlot::SetAxes(ImAxis_X1, ImAxis_Y2);
                    line.LineColor = COL_OMEGA;
                    ImPlot::PlotLine("ω (rad/s)", &d->t, &d->omega, n, line);
                    ImPlot::EndPlot();
                }
                ImGui::EndTabItem();
            }

            if(ImGui::BeginTabItem("Energy")){
                if(ImPlot::BeginPlot("##energy", fill)){
                    ImPlot::SetupAxes("time (s)", "energy (J)", ImPlotAxisFlags_None, fitY);
                    ImPlot::SetupAxisLimits(ImAxis_X1, tMin, tMax, ImPlotCond_Always);
                    ImPlot::SetupLegend(ImPlotLocation_North, legendFlags);

                    line.LineColor = COL_KINETIC;
                    ImPlot::PlotLine("Kinetic", &d->t, &d->kinetic, n, line);
                    line.LineColor = COL_POTENTIAL;
                    ImPlot::PlotLine("Potential", &d->t, &d->potential, n, line);
                    line.LineColor = COL_TOTAL;
                    ImPlot::PlotLine("Total", &d->t, &d->total, n, line);
                    ImPlot::EndPlot();
                }
                ImGui::EndTabItem();
            }

            if(ImGui::BeginTabItem("Phase space (θ vs ω)")){
                if(ImPlot::BeginPlot("##phase", fill)){
                    ImPlot::SetupAxes("θ (deg)", "ω (rad/s)", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    ImPlot::SetupLegend(ImPlotLocation_North, legendFlags);

                    line.LineColor = COL_THETA;
                    ImPlot::PlotLine("trajectory", &d->theta, &d->omega, n, line);

                    // Current state as a dot
                    const Sample& now = history.fromEnd(0);
                    ImPlotSpec dot;
                    dot.Marker          = ImPlotMarker_Circle;
                    dot.MarkerSize      = 5.0f;
                    dot.MarkerFillColor = COL_POTENTIAL;
                    dot.MarkerLineColor = COL_POTENTIAL;
                    ImPlot::PlotScatter("now", &now.theta, &now.omega, 1, dot);
                    ImPlot::EndPlot();
                }
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

// ---------------------------------------------------------------------------
// Right-bottom: inputs to change the pendulum and the environment
// ---------------------------------------------------------------------------

// ImGui sliders work on float/int; this one edits a double directly
static bool sliderDouble(const char* label, double* value, double min, double max, const char* format){
    return ImGui::SliderScalar(label, ImGuiDataType_Double, value, &min, &max, format,
                               ImGuiSliderFlags_AlwaysClamp);
}

void drawControlsPanel(Simulation& sim, const Section& section, float displayHeight){
    if(beginSectionWindow("##controls", section, displayHeight)){
        sectionTitle("CONTROLS");

        SinglePendulum& p = sim.pendulum;
        float spacing = ImGui::GetStyle().ItemSpacing.x;
        float half    = (ImGui::GetContentRegionAvail().x - spacing) * 0.5f;

        if(ImGui::Button(sim.running ? "Pause   [Space]" : "Play   [Space]", ImVec2(half, 0))){
            sim.running = !sim.running;
        }
        ImGui::SameLine();
        if(ImGui::Button("Reset   [R]", ImVec2(half, 0))){
            sim.reset();
        }

        // Changing a physical parameter changes the energy, so take the current energy
        // as the new reference for ΔE
        bool physicsChanged = false;

        // Two columns: pendulum on the left, environment + simulation on the right
        if(ImGui::BeginTable("controls", 2, ImGuiTableFlags_SizingStretchSame)){
            ImGui::TableNextRow();

            // ---- Left column ----
            ImGui::TableNextColumn();
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.55f); // slider left, label right

            ImGui::SeparatorText("Pendulum");
            physicsChanged |= sliderDouble("String length L", &p.length, 0.1, 5.0, "%.2f m");
            physicsChanged |= sliderDouble("Bob mass m", &p.mass, 0.1, 20.0, "%.2f kg");
            ImGui::SetItemTooltip("Mass doesn't change the motion of a simple pendulum,\nonly its energy and the string tension.");

            // Start conditions restart the simulation from the new values
            if(sliderDouble("Start angle θ0", &sim.initialAngleDeg, -179.9, 179.9, "%.1f°")) sim.reset();
            ImGui::SetItemTooltip("Changing this restarts the simulation.\nYou can also drag the bob in the animation.");
            if(sliderDouble("Start velocity ω0", &sim.initialOmega, -10.0, 10.0, "%.2f rad/s")) sim.reset();
            ImGui::SetItemTooltip("Changing this restarts the simulation.");

            ImGui::PopItemWidth();

            // ---- Right column ----
            ImGui::TableNextColumn();
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.55f);

            ImGui::SeparatorText("Environment");
            physicsChanged |= sliderDouble("Gravity g", &p.g, 0.1, 30.0, "%.2f m/s²");

            // Gravity presets
            struct Preset { const char* name; double g; };
            static const Preset presets[] = { {"Earth", 9.81}, {"Moon", 1.62}, {"Mars", 3.71}, {"Jupiter", 24.79} };
            float presetWidth = (ImGui::GetContentRegionAvail().x - 3 * spacing) / 4.0f;
            for(int i = 0; i < 4; i++){
                if(i > 0) ImGui::SameLine();
                if(ImGui::Button(presets[i].name, ImVec2(presetWidth, 0))){
                    p.g = presets[i].g;
                    physicsChanged = true;
                }
                ImGui::SetItemTooltip("g = %.2f m/s²", presets[i].g);
            }
            physicsChanged |= sliderDouble("Damping b", &p.damping, 0.0, 2.0, "%.3f 1/s");
            ImGui::SetItemTooltip("Air resistance / friction. 0 = energy is conserved.");

            ImGui::SeparatorText("Simulation");
            ImGui::SliderFloat("Speed", &sim.speed, 0.1f, 3.0f, "%.2fx", ImGuiSliderFlags_AlwaysClamp);
            ImGui::SliderFloat("Graph window", &sim.graphWindow, 2.0f, 60.0f, "%.0f s", ImGuiSliderFlags_AlwaysClamp);

            ImGui::PopItemWidth();
            ImGui::EndTable();
        }

        if(physicsChanged){
            sim.initialEnergy = p.kineticEnergy() + p.potentialEnergy();
        }

        ImGui::Spacing();
        ImGui::TextDisabled("Tip: Ctrl+Click a slider to type an exact value. Hover a control for details.");
    }
    ImGui::End();
}
