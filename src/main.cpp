#define SDL_MAIN_HANDLED
#define SDL_STATIC

#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#include "TimVectorMath.h"
#define IMGUI_USER_CONFIG "ImGuiConfig.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define IMGUI_IMPL_OPENGL_ES2
#else
// If not web then windows only for now.
#include "gl3w.cpp"
#endif

#include <SDL.h>
#include <imgui.h>
#include <implot.h>

#include "imgui.cpp"
#include "imgui_draw.cpp"
#include "imgui_tables.cpp"
#include "imgui_widgets.cpp"

#include "implot.cpp"
#include "implot_items.cpp"

#include "imgui_impl_sdl2.cpp"
#include "imgui_impl_opengl3.cpp"

#include "AnymUtil.h"
#include "Agent.h"
#include "Array.h"
#include "Camera2D.h"
#include "DebugInfo.h"
#include "External.h"
#include "InputHandler.h"
#include "Linalg.h"
#include "TMath.h"
#include "Memory.h"
#include "Mesh2D.h"
#include "ParticleSystem.h"
#include "Shader.h"
#include "SimulationScreen.h"
#include "SimulationSettings.h"
#include "ThreadPool.h"

#include "Agent.cpp"
#include "Array.h"
#include "Camera2D.cpp"
#include "DebugInfo.cpp"
#include "External.cpp"
#include "InputHandler.cpp"
#include "Linalg.cpp"
#include "Memory.cpp"
#include "Mesh2D.cpp"
#include "ParticleSystem.cpp"
#include "Shader.cpp"
#include "SimulationScreen.cpp"
#include "SimulationSettings.h"
#include "TimVectorMath.cpp"
#include "Window.cpp"
#include "World.cpp"

#if 0
#include <direct.h>  

static void
PrintWorkingDirectory()
{
    char cwd[1024];  

    if (_getcwd(cwd, sizeof(cwd))) {
        std::cout << "Current working directory: " << cwd << std::endl;
    } else {
        std::cerr << "Error retrieving the current working directory." << std::endl;
    }
}

int main(int argc, char** argv) 
{
    // Initialize random seed once. This will be replaced by my own rng.
    srand((time(nullptr)));

#undef CreateWindow
    Window* window = CreateWindow(1280, 720);
    if(!window) 
    {
        return -1;
    }
    window->fps = 60;

    PrintWorkingDirectory();

    SimulationScreen screen;
    InitSimulationScreen(&screen);
    while (window->running) 
    {
        WindowBegin(window);
        UpdateSimulationScreen(&screen, window);
        WindowEnd(window);
    }
    DestroyWindow(window);

    return 0;
}
#endif

static Window* window;
static SimulationScreen* screen;

// This function will be called on each frame
void main_loop() {
    if (!window->running) {
        emscripten_cancel_main_loop();
        return;
    }

    WindowBegin(window);
    UpdateSimulationScreen(screen, window);
    WindowEnd(window);
}

int main(int argc, char** argv) 
{
    // Initialize random seed once.
    srand((time(nullptr)));

    #undef CreateWindow
    window = CreateWindow(1280, 720);
    if (!window) {
        return -1;
    }
    window->fps = 60;

    screen = new SimulationScreen();

    InitSimulationScreen(screen);

    // Start the main loop with Emscripten
    emscripten_set_main_loop(main_loop, 0, 1);

    DestroyWindow(window);
    delete screen;

    return 0;
}

