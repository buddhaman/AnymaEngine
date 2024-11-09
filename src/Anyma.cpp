#define SDL_MAIN_HANDLED
#define SDL_STATIC

#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#include "TimVectorMath.h"
#define IMGUI_USER_CONFIG "ImGuiConfig.h"

#include <SDL.h>
#include <gl3w.h>
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

#include "gl3w.cpp"

#include "AnymUtil.h"
#include "Array.h"
#include "TiltCamera.h"
#include "Camera2D.h"
#include "DebugInfo.h"
#include "External.h"
#include "InputHandler.h"
#include "Linalg.h"
#include "Genome.h"
#include "TMath.h"
#include "TString.h"
#include "Memory.h"
#include "Mesh2D.h"
#include "ParticleSystem.h"
#include "Shader.h"
#include "Texture.h"
#include "Renderer.h"
#include "Skeleton.h"
#include "Agent.h"
#include "SimulationScreen.h"
#include "SimulationSettings.h"
#include "ThreadPool.h"
#include "EditorScreen.h"

#include "Agent.cpp"
#include "Array.h"
#include "Camera2D.cpp"
#include "TiltCamera.cpp"
#include "DebugInfo.cpp"
#include "External.cpp"
#include "InputHandler.cpp"
#include "Linalg.cpp"
#include "Genome.cpp"
#include "Memory.cpp"
#include "Mesh2D.cpp"
#include "ParticleSystem.cpp"
#include "Shader.cpp"
#include "Texture.cpp"
#include "Renderer.cpp"
#include "SimulationScreen.cpp"
#include "Skeleton.cpp"
#include "SimulationSettings.h"
#include "TimVectorMath.cpp"
#include "Window.cpp"
#include "World.cpp"
#include "EditorScreen.cpp"

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
    srand((U32)(time(nullptr)));

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

    EditorScreen editor;
    InitEditorScreen(&editor);

    while (window->running) 
    {
        WindowBegin(window);
        switch(global_settings.current_phase)
        {
            case GamePhase_SimulationScreen: 
            {
                UpdateSimulationScreen(&screen, window);
            } break;

            case GamePhase_EditorScreen: 
            {
                UpdateEditorScreen(&editor, window);
            } break;
        }
        WindowEnd(window);
    }
    DestroyWindow(window);

    return 0;
}
