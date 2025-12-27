#pragma once

#include "Mesh2D.h"
#include "Shader.h"
#include "Window.h"
#include "Camera2D.h"
#include "Texture.h"
#include "Renderer.h"
#include "Soup.h"
#include "Array.h"
#include "SimpleTest.h"

struct Agent;  // Forward declaration

struct EditorScreen
{
    MemoryArena* editor_arena;
    Camera2D cam;

    TextureAtlas* atlas;

    TiltedRenderer* renderer;

    Agent* agent;

    Soup* soup;

    // Simple learning test
    SimpleTestSoup* simple_test;
    int simple_test_step;
    int simple_test_current_input;
    DynamicArray<R32> simple_test_accuracy;
    bool simple_test_playing;

    R32 time = 0.0f;
    
    // Tracking arrays for Soup visualization
    DynamicArray<R32> active_neuron_history;
    DynamicArray<R32> avg_bias_history;
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
    
    // Chunk accuracy history for graphing (short-term, last 200 chunks)
    DynamicArray<R32> chunk_accuracy_history;
    DynamicArray<R32> tokens_per_chunk_history;
    int total_chunks_processed = 0;
    
    // Long-term accuracy tracking (sampled every N chunks, kept indefinitely)
    int longterm_sample_rate = 10;      // Sample every N chunks
    int longterm_sample_counter = 0;
    DynamicArray<R32> longterm_accuracy_history;
    DynamicArray<R32> longterm_chunk_numbers;  // X-axis: which chunk number
    float longterm_accuracy_sum = 0.0f;        // Accumulate accuracy over sample period
    int longterm_samples_in_period = 0;
    
    // Prediction history - last N predictions vs actual targets
    static constexpr int prediction_history_size = 50;
    DynamicArray<char> predicted_chars;   // What the model predicted
    DynamicArray<char> target_chars;      // What the target was
    int last_predicted_index = -1;        // Index of last predicted character
    float last_softmax_confidence = 0.0f; // Confidence of the prediction
    float last_dopamine = 0.0f;           // Last dopamine signal applied

    // Chat interface
    char chat_input_buffer[256] = {0};    // Input for seeding the network
    DynamicArray<char> chat_output;       // Generated output from the network
    bool chat_show_window = false;        // Whether to show chat window
    int chat_tokens_to_generate = 20;     // How many tokens to generate

    EditorScreen() : active_neuron_history(200), avg_bias_history(200), firing_rate_history(200),
                     chunk_accuracy_history(200), tokens_per_chunk_history(200),
                     longterm_accuracy_history(10000), longterm_chunk_numbers(10000),
                     predicted_chars(50), target_chars(50),
                     simple_test_accuracy(1000),
                     chat_output(1000)
    {
        simple_test = nullptr;
        simple_test_step = 0;
        simple_test_current_input = 0;
        simple_test_playing = false;
    }
};

int
UpdateEditorScreen(EditorScreen* editor, Window* window);

void
InitEditorScreen(EditorScreen* screen);
