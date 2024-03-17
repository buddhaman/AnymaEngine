#pragma once

#include "World.h"
#include "Mesh2D.h"
#include "Shader.h"
#include "Window.h"
#include "SimulationSettings.h"

struct SimulationScreen
{
    World* world;
    Mesh2D dynamic_mesh;

    Agent* selected = nullptr;

    MemoryArena* world_arena;

    Camera2D cam;
    Shader shader;
    U32 transform_loc;

    DynamicArray<R32> update_times;

    int updates_per_frame = 1;
    R32 time = 0.0f;

    bool show_chunks = true;
    bool show_debug_window = true;
    SimulationSettings settings;
    
    SimulationScreen() : update_times(120) {};
};

void
UpdateSimulationScreen(SimulationScreen* screen, Window* window);

void
RestartWorld(SimulationScreen* screen);

void
InitSimulationScreen(SimulationScreen* screeen);