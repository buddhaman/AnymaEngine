#include <SDL.h>
#include <GL/glew.h>
#include <imgui.h>
#include <implot.h>

#include "AnymUtil.h"
#include "Array.h"
#include "TimVectorMath.h"

#include "Mesh2D.h"
#include "Camera2D.h"
#include "Window.h"
#include "InputHandler.h"
#include "Shader.h"
#include "DebugInfo.h"

#include "World.h"
#include "SimulationScreen.h"

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
