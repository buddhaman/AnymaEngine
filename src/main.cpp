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
