#include <iostream>
#include <include/Canvas.h>
#include <include/Pendulum.h>
#include <include/Drawing.h>
#include <include/Panels.h>
#include <include/keyboard.h>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <glad/gl.h> //used for loading Opengl functions
#include <GLFW/glfw3.h> // Used for creating windows, handling keyboard input,
                        // handling mouse input, Managing the opengl context.
#include <imgui.h>                // Dear ImGui: text, buttons, sliders
#include <imgui_impl_glfw.h>      // ImGui <-> GLFW (input, window)
#include <imgui_impl_opengl3.h>   // ImGui <-> OpenGL (rendering)
#include <implot.h>               // ImPlot: live graphs on top of ImGui

void glfw_error_callback(int error, const char* description ){
    std::cerr << "GLFW Error [" << error << "]: " << description << '\n';
}

// Shrinks a section by 'pad' pixels on every side, so neighbouring sections
// leave a visible gap (border) between them
static Section inset(const Section& s, int pad){
    return { s.x + pad,
             s.y + pad,
             std::max(0, s.width  - 2 * pad),
             std::max(0, s.height - 2 * pad) };
}

// Divides the window into the 4 sections:
//
//   +--------------------+--------------------+
//   |                    |       graph        |
//   |     animation      |       (50%)        |
//   |       (70%)        |                    |
//   |                    +--------------------+
//   +--------------------+      controls      |
//   |   working (30%)    |       (50%)        |
//   +--------------------+--------------------+
//         left 50%              right 50%
//
// Called every frame with the current framebuffer size, so the layout
// follows the window when it is resized.
Layout computeLayout(int windowWidth, int windowHeight){
    int leftWidth  = static_cast<int>(windowWidth * LEFT_WIDTH_RATIO);
    int rightWidth = windowWidth - leftWidth;

    // Left side: working sits at the bottom, animation above it
    int animationHeight = static_cast<int>(windowHeight * ANIMATION_HEIGHT_RATIO);
    int workingHeight   = windowHeight - animationHeight;

    // Right side: controls sit at the bottom, graph above it
    int graphHeight    = static_cast<int>(windowHeight * GRAPH_HEIGHT_RATIO);
    int controlsHeight = windowHeight - graphHeight;

    const int pad = SECTION_GAP / 2; // half gap per side -> full gap between sections

    Layout layout;
    layout.working   = inset({0,         0,             leftWidth,  workingHeight},   pad);
    layout.animation = inset({0,         workingHeight, leftWidth,  animationHeight}, pad);
    layout.controls  = inset({leftWidth, 0,             rightWidth, controlsHeight},  pad);
    layout.graph     = inset({leftWidth, controlsHeight, rightWidth, graphHeight},    pad);
    return layout;
}

// Restricts drawing to one section and clears it with its background color.
// Anything drawn after this call (until the next drawSection) lands inside the section.
void drawSection(const Section& section, const Color& color){
    glViewport(section.x, section.y, section.width, section.height);
    glScissor(section.x, section.y, section.width, section.height);

    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

int Canvas(){
   
    //Error callback 
    glfwSetErrorCallback(glfw_error_callback);

    // Initializing the GLFW
    if(!glfwInit()){
        std::cerr << "Failed to initilaize GLFW lib\n";
        return -1;
    }

    // Configuringg OpenGL version (eg., 3.3 Core profile)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open the window maximized so the GUI takes the full screen (title bar kept, so it can be closed)
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    // Create the window canvas and opengl context
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    GLFWwindow* window = glfwCreateWindow( mode -> width, mode -> height, "N-Pendulum", nullptr , nullptr);
    if(!window){
        std::cerr << "GLFW initialization failed!\n";
        glfwTerminate();
        return -1;

    }

    // Make the window's context current
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // vsync: limit rendering to the monitor's refresh rate

    // Initializing the glad
    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)){
        std::cerr << "Failed to Initialize GLAD\n";

        glfwDestroyWindow(window);
        glfwTerminate();

        return -1;

    }

    // Initializing Dear ImGui + ImPlot
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGui::GetIO().IniFilename = nullptr; // don't write an imgui.ini file next to the exe

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Monitor scale (1.25 on a 125% display) so text and widgets aren't tiny
    float dpiScale, dpiScaleY;
    glfwGetWindowContentScale(window, &dpiScale, &dpiScaleY);
    setupStyle(dpiScale);

    // The pendulum simulation shown in all four sections
    Simulation sim;
    sim.reset();

    // render loop
    while (!glfwWindowShouldClose(window)){

        // Actual drawable size in pixels (can differ from window size on scaled displays)
        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

        // Window is minimized -> nothing to draw, just wait for events
        if(fbWidth == 0 || fbHeight == 0){
            glfwWaitEvents();
            continue;
        }

        // ---- Start a new ImGui frame ----
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuiIO& io = ImGui::GetIO();

        // ---- Input + physics ----
        handleKeyboard(sim);
        sim.update(io.DeltaTime);

        // ---- Build the UI of each section ----
        // ImGui works in window coordinates, so it gets its own layout from the window size
        Layout ui = computeLayout((int)io.DisplaySize.x, (int)io.DisplaySize.y);
        drawAnimationPanel(sim, ui.animation, io.DisplaySize.y);
        drawWorkingPanel(sim, ui.working, io.DisplaySize.y);
        drawGraphPanel(sim, ui.graph, io.DisplaySize.y);
        drawControlsPanel(sim, ui.controls, io.DisplaySize.y);
        ImGui::Render();

        Layout layout = computeLayout(fbWidth, fbHeight);

        // Clear the whole window with the border color first;
        // the gaps left between sections become the border lines
        glDisable(GL_SCISSOR_TEST);
        glViewport(0, 0, fbWidth, fbHeight);
        glClearColor(Colors::Border.r, Colors::Border.g, Colors::Border.b, Colors::Border.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Enable scissor testing so glClear only clears the active section
        glEnable(GL_SCISSOR_TEST);

        // Section backgrounds (the content is drawn on top by ImGui below)
        drawSection(layout.animation, Colors::Animation); // left-top
        drawSection(layout.working,   Colors::Working);   // left-bottom
        drawSection(layout.graph,     Colors::Graph);     // right-top
        drawSection(layout.controls,  Colors::Controls);  // right-bottom

        glDisable(GL_SCISSOR_TEST);

        // Draw all the ImGui windows (animation, working, graph, controls)
        glViewport(0, 0, fbWidth, fbHeight);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        //swap front/back buffers and poll input events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //6. Clean up resources
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();


    return 0;
}


int main(){
    return Canvas();
}
