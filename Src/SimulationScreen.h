#pragma once

#include "World.h"
#include "Mesh2D.h"
#include "Shader.h"
#include "Window.h"
#include "SimulationSettings.h"
#include "ThreadPool.h"
#include "Texture.h"
#include "Agent.h"

struct SimulationScreen
{
    World* world;
    Mesh2D dynamic_mesh;

    R32 extra_selection_radius = 1.0f;
    Agent* hovered_agent = nullptr;
    Agent* selected_agent = nullptr;

    MemoryArena* screen_arena;
    MemoryArena* world_arena;

    Camera2D cam;
    Shader shader;

    TiltedRenderer* renderer;

    DynamicArray<R32> update_times;

    int track_population_per = 6;
    DynamicArray<R32> num_carnivores;
    DynamicArray<R32> num_herbivores;

    bool isPaused = false;
    int updates_per_frame = 1;
    R32 time = 0.0f;

    bool show_cell_occupancy = false;
    bool show_debug_window = true;
    bool show_edit_window = true;
    bool show_statistic_window = true;
    bool show_control_window = true;

    bool show_health = true;
    bool show_grid = true;

    ColorOverlay overlay = ColorOverlay_AgentType;

    ThreadPool* thread_pool;

    TextureAtlas* atlas;  
    AtlasRegion* square;
    
    SimulationScreen() : update_times(120), num_carnivores(120), num_herbivores(120) {};
};

int
UpdateSimulationScreen(SimulationScreen* screen, Window* window);

void
RestartWorld(SimulationScreen* screen);

void
InitSimulationScreen(SimulationScreen* screeen);