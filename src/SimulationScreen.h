#pragma once

#include "World.h"
#include "Mesh2D.h"
#include "Shader.h"
#include "Window.h"

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

    int updates_per_frame = 0;
    R32 time = 0.0f;
    
    SimulationScreen() : update_times(120) {};
};

void
UpdateSimulationScreen(SimulationScreen* screen, Window* window);

void
InitSimulationScreen(SimulationScreen* screeen);