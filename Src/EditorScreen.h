#pragma once

#include "Mesh2D.h"
#include "Shader.h"
#include "Window.h"
#include "Camera2D.h"
#include "Texture.h"
#include "Renderer.h"
#include "Soup.h"
#include "Array.h"

struct Agent;  // Forward declaration

struct EditorScreen
{
    MemoryArena* editor_arena;
    Camera2D cam;

    TextureAtlas* atlas;

    TiltedRenderer* renderer;

    Agent* agent;

    Soup* soup;

    R32 time = 0.0f;
    
    // Tracking arrays for Soup visualization
    DynamicArray<R32> active_neuron_history;
    DynamicArray<R32> avg_threshold_history;
    DynamicArray<R32> firing_rate_history;
    int track_history_per = 1;
    int history_track_counter = 0;
    
    // Run controls
    bool soup_playing = false;
    int soup_steps_per_frame = 1;
    
    EditorScreen() : active_neuron_history(200), avg_threshold_history(200), firing_rate_history(200) {}
};

int
UpdateEditorScreen(EditorScreen* editor, Window* window);

void
InitEditorScreen(EditorScreen* screen);
