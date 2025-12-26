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
    
    // Token training
    const char* training_text;
    int training_text_length;
    int current_token_index;
    int token_window_size;
    int num_chars;  // Size of one-hot encoding (26 letters + punctuation)
    int num_input_neurons;
    int num_actuator_neurons;  // 10 neurons before output
    int num_output_neurons;
    int steps_since_actuator;
    bool waiting_for_actuator;
    float last_accuracy;
    
    // Delayed reward tracking (chunk-based)
    int chunk_size = 100;           // Iterations per chunk
    int current_chunk_iteration = 0;
    int chunk_tokens_processed = 0;
    float chunk_accuracy_sum = 0.0f;
    float last_chunk_accuracy = 0.0f;
    float current_chunk_accuracy = 0.0f;
    bool dopamine_released_this_chunk = false;
    
    // Chunk accuracy history for graphing
    DynamicArray<R32> chunk_accuracy_history;
    DynamicArray<R32> tokens_per_chunk_history;
    int total_chunks_processed = 0;
    
    EditorScreen() : active_neuron_history(200), avg_threshold_history(200), firing_rate_history(200),
                     chunk_accuracy_history(200), tokens_per_chunk_history(200) {}
};

int
UpdateEditorScreen(EditorScreen* editor, Window* window);

void
InitEditorScreen(EditorScreen* screen);
