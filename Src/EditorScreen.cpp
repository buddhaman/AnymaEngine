#include <imgui.h>
#include <implot.h>
#include <gl3w.h>
#include <cstring>

#include "AnymUtil.h"
#include "EditorScreen.h"
#include "SimulationSettings.h"
#include "Lucide.h"

#include "Agent.h"

constexpr R32 speed = 0.34f;

// Character set: a-z (26) + space, period, comma, question mark, exclamation mark = 31
constexpr int NUM_CHARS = 31;
const char* CHAR_SET = "abcdefghijklmnopqrstuvwxyz .,?!";

// Test training text
const char* TRAINING_TEXT = "the quick brown fox jumps over the lazy dog. hello world! how are you? this is a test sentence.";

// One-hot encode a character
int CharToIndex(char c)
{
    if(c >= 'a' && c <= 'z')
        return c - 'a';
    if(c == ' ') return 26;
    if(c == '.') return 27;
    if(c == ',') return 28;
    if(c == '?') return 29;
    if(c == '!') return 30;
    return 26; // Default to space for unknown chars
}

char IndexToChar(int idx)
{
    if(idx >= 0 && idx < NUM_CHARS)
        return CHAR_SET[idx];
    return ' ';
}

// Get one-hot encoding for a character
void OneHotEncode(char c, int* output, int size)
{
    for(int i = 0; i < size; i++)
        output[i] = 0;
    int idx = CharToIndex(c);
    if(idx >= 0 && idx < size)
        output[idx] = 1;
}

// Measure accuracy between predicted and target one-hot
float MeasureAccuracy(int* predicted, int* target, int size)
{
    int matches = 0;
    for(int i = 0; i < size; i++)
    {
        if(predicted[i] == target[i])
            matches++;
    }
    return (float)matches / (float)size;
}

// Softmax-based prediction: use neuron activations to predict character
// output_start is the index into soup->neurons (processing neurons)
// Returns the index of the predicted character and the softmax confidence
int SoftmaxPredict(Soup* soup, int output_start, int num_outputs, float* out_confidence)
{
    // Get activations from output neurons (in processing layer)
    float max_activation = -1e30f;
    int best_index = 0;
    
    // Find max for numerical stability
    for(int i = 0; i < num_outputs && (output_start + i) < soup->neurons.size; i++)
    {
        float act = soup->neurons[output_start + i].activation;
        if(act > max_activation)
        {
            max_activation = act;
            best_index = i;
        }
    }
    
    // Compute softmax probabilities
    float sum_exp = 0.0f;
    float target_exp = 0.0f;
    for(int i = 0; i < num_outputs && (output_start + i) < soup->neurons.size; i++)
    {
        float act = soup->neurons[output_start + i].activation;
        float exp_val = expf(act - max_activation);  // Subtract max for numerical stability
        sum_exp += exp_val;
        if(i == best_index)
        {
            target_exp = exp_val;
        }
    }
    
    // Confidence is the softmax probability of the best index
    if(out_confidence && sum_exp > 0.0f)
    {
        *out_confidence = target_exp / sum_exp;
    }
    
    return best_index;
}

// Compute softmax probability for a specific target index (for reward calculation)
// output_start is the index into soup->neurons (processing neurons)
float SoftmaxProbability(Soup* soup, int output_start, int num_outputs, int target_index)
{
    // Find max activation for numerical stability
    float max_activation = -1e30f;
    for(int i = 0; i < num_outputs && (output_start + i) < soup->neurons.size; i++)
    {
        float act = soup->neurons[output_start + i].activation;
        if(act > max_activation)
        {
            max_activation = act;
        }
    }
    
    // Compute softmax denominator and target numerator
    float sum_exp = 0.0f;
    float target_exp = 0.0f;
    for(int i = 0; i < num_outputs && (output_start + i) < soup->neurons.size; i++)
    {
        float act = soup->neurons[output_start + i].activation;
        float exp_val = expf(act - max_activation);
        sum_exp += exp_val;
        if(i == target_index)
        {
            target_exp = exp_val;
        }
    }
    
    if(sum_exp > 0.0f)
    {
        return target_exp / sum_exp;
    }
    return 0.0f;
}

// Helper to set input layer from one-hot encoded tokens
void SetInputFromContext(Soup* soup, const char* text, int text_length, int current_index, int window_size)
{
    int input_idx = 0;
    for(int token = 0; token < window_size; token++)
    {
        int text_idx = current_index - window_size + 1 + token;
        if(text_idx < 0) text_idx = 0;
        if(text_idx >= text_length) text_idx = text_length - 1;
        
        char c = text[text_idx];
        int char_idx = CharToIndex(c);
        
        // Set one-hot encoding in input layer (soup->inputs is Array<Neuron>)
        for(int i = 0; i < NUM_CHARS && input_idx < (int)soup->inputs.size; i++)
        {
            soup->inputs[input_idx].state = (i == char_idx) ? 1 : 0;
            input_idx++;
        }
    }
}
constexpr R32 turn_speed = 0.01f;
constexpr int max_joints = 64;
static R32 walk_radius = speed/sinf(turn_speed);

bool EditFloatArray(const char* labelPrefix, float* array, int count, float min, float max)
{
    bool changed = false;
    for (int i = 0; i < count; ++i)
    {
        char label[32];
        snprintf(label, sizeof(label), "%s[%d]", labelPrefix, i);
        changed |= ImGui::SliderFloat(label, &array[i], min, max);
    }
    return changed;
}

bool EditBoolArray(const char* labelPrefix, uint32_t* array, int count)
{
    bool changed = false;
    for (int i = 0; i < count; ++i)
    {
        char label[32];
        snprintf(label, sizeof(label), "%s[%d]", labelPrefix, i);
        bool value = array[i];
        if (ImGui::Checkbox(label, &value))
        {
            changed = true;
            array[i] = static_cast<uint32_t>(value);
        }
    }
    return changed;
}

void UpdateTokens(EditorScreen* editor)
{
    Soup* soup = editor->soup;
    int output_start = (int)soup->neurons.size - editor->num_output_neurons;
    
    // Get target for this step (next character after current position)
    char target_char = ' ';
    int target_index = 26;  // space
    if(editor->current_token_index < editor->training_text_length - 1)
    {
        target_char = editor->training_text[editor->current_token_index + 1];
        target_index = CharToIndex(target_char);
    }
    
    // Run soup update WITHOUT eligibility update (we'll do supervised eligibility)
    UpdateSoup(*soup, false);
    
    // Measure prediction BEFORE setting correct output
    float confidence = 0.0f;
    int predicted_index = SoftmaxPredict(soup, output_start, editor->num_output_neurons, &confidence);
    char predicted_char = IndexToChar(predicted_index);
    float softmax_prob = SoftmaxProbability(soup, output_start, editor->num_output_neurons, target_index);
    
    editor->last_predicted_index = predicted_index;
    editor->last_softmax_confidence = confidence;
    editor->last_accuracy = softmax_prob;
    
    // SUPERVISED LEARNING: Set output neurons to CORRECT values
    // This is the key fix - we build eligibility traces for correct output, not random output
    for(int i = 0; i < editor->num_output_neurons && (output_start + i) < soup->neurons.size; i++)
    {
        soup->neurons[output_start + i].state = (i == target_index) ? 1 : 0;
    }
    
    // NOW update eligibility based on correct output pattern
    // This builds traces: "these input patterns should produce this correct output"
    UpdateEligibility(*soup);
    
    // Apply dopamine - always positive for supervised learning
    // Stronger signal when prediction was wrong (more to learn)
    float dopamine = 1.0f;  // Always reinforce correct pattern
    ApplyDopamine(*soup, dopamine);
    editor->last_dopamine = dopamine;
    
    // Store prev states for next step
    StorePrevStates(*soup);
    
    // Track prediction accuracy (for monitoring, not for learning)
    editor->chunk_accuracy_sum += (predicted_index == target_index) ? 1.0f : 0.0f;
    editor->chunk_tokens_processed++;
    
    // Add to prediction history
    if(editor->predicted_chars.size >= editor->prediction_history_size)
    {
        editor->predicted_chars.Shift(-1);
        editor->target_chars.Shift(-1);
        editor->predicted_chars[editor->predicted_chars.size - 1] = predicted_char;
        editor->target_chars[editor->target_chars.size - 1] = target_char;
    }
    else
    {
        editor->predicted_chars.PushBack(predicted_char);
        editor->target_chars.PushBack(target_char);
    }
    
    // Move to next token every step (no actuator waiting)
    editor->current_token_index++;
    if(editor->current_token_index >= editor->training_text_length - 1)
    {
        editor->current_token_index = 0;  // Wrap around
    }
    
    // Set new input for next step
    SetInputFromContext(soup, editor->training_text, editor->training_text_length, 
                       editor->current_token_index, editor->token_window_size);
    
    // Track chunk statistics (for visualization)
    editor->current_chunk_iteration++;
    if(editor->current_chunk_iteration >= editor->chunk_size)
    {
        // Calculate chunk accuracy for display (now tracking exact match rate)
        if(editor->chunk_tokens_processed > 0)
        {
            editor->current_chunk_accuracy = editor->chunk_accuracy_sum / (float)editor->chunk_tokens_processed;
        }
        else
        {
            editor->current_chunk_accuracy = 0.0f;
        }
        
        // Track history for plotting
        const int max_history = 200;
        if(editor->chunk_accuracy_history.size >= max_history)
        {
            editor->chunk_accuracy_history.Shift(-1);
            editor->tokens_per_chunk_history.Shift(-1);
        }
        editor->chunk_accuracy_history.PushBack(editor->current_chunk_accuracy);
        editor->tokens_per_chunk_history.PushBack((R32)editor->chunk_tokens_processed);
        
        editor->last_chunk_accuracy = editor->current_chunk_accuracy;
        editor->total_chunks_processed++;
        
        // Long-term tracking
        editor->longterm_accuracy_sum += editor->current_chunk_accuracy;
        editor->longterm_samples_in_period++;
        editor->longterm_sample_counter++;
        
        if(editor->longterm_sample_counter >= editor->longterm_sample_rate)
        {
            float avg_accuracy = editor->longterm_accuracy_sum / (float)editor->longterm_samples_in_period;
            if(editor->longterm_accuracy_history.size < editor->longterm_accuracy_history.capacity)
            {
                editor->longterm_accuracy_history.PushBack(avg_accuracy);
                editor->longterm_chunk_numbers.PushBack((R32)editor->total_chunks_processed);
            }
            editor->longterm_sample_counter = 0;
            editor->longterm_accuracy_sum = 0.0f;
            editor->longterm_samples_in_period = 0;
        }
        
        // Reset chunk counters
        editor->current_chunk_iteration = 0;
        editor->chunk_tokens_processed = 0;
        editor->chunk_accuracy_sum = 0.0f;
    }
}

// Hidden function - not called
bool EditCreatureWindowHidden(PhenoType* pheno)
{
    bool changed = false;
    // Edit the number of backbones with clamping
    ImGui::Text("Backbones");
    if (ImGui::InputInt("Number of Backbones", &pheno->n_backbones)) 
    {
        pheno->n_backbones = Clamp(1, pheno->n_backbones, 12);
        changed = true;
    }

    // Edit backbone radii
    ImGui::Text("Backbone Radii");
    changed |= EditFloatArray("Radius", pheno->backbone_radius.v, pheno->n_backbones, 0.1f, 6.0f);

    // Edit has_leg flags
    ImGui::Text("Legs");
    changed |= EditBoolArray("Has Leg", pheno->has_leg.data, pheno->n_backbones);

    // Edit knee sizes
    ImGui::Text("Knee Sizes");
    changed |= EditFloatArray("Knee Size", pheno->knee_size.v, pheno->n_backbones, 0.1f, 6.0f);

    // Edit foot sizes
    ImGui::Text("Foot Sizes");
    changed |= EditFloatArray("Foot Size", pheno->foot_size.v, pheno->n_backbones, 0.1f, 6.0f);

    ImGui::Text("Step radius");
    changed |= EditFloatArray("Step radius", pheno->step_radius.v, pheno->n_backbones, 0.1f, 6.0f);

    // Edit elbow size
    ImGui::Text("Limb Sizes");
    changed |= ImGui::SliderFloat("Elbow Size", &pheno->elbow_size, 0.1f, 6.0f);

    // Edit hand size
    changed |= ImGui::SliderFloat("Hand Size", &pheno->hand_size, 0.1f, 6.0f);

    Vec4 color = ColorToVec4(pheno->color);
    changed |= ImGui::ColorPicker3("Color", color.elements);
    pheno->color = Vec4ToColor(color);
    return changed;
}

void EditSoupWindow(EditorScreen* editor)
{
    if(!editor->soup) return;
    
    Soup* soup = editor->soup;
    MemoryArena* arena = editor->editor_arena;
    
    ImGui::Begin("Soup Editor");
    
    // Create new Soup
    static int new_num_processing = 1000;
    static int new_num_connections = 10;
    static int new_context_length = 10;
    ImGui::Text("Create New Soup");
    ImGui::InputInt("Context Length (tokens)", &new_context_length);
    ImGui::InputInt("Processing Neurons", &new_num_processing);
    ImGui::InputInt("Connections per Neuron", &new_num_connections);
    new_context_length = Clamp(1, new_context_length, 20);
    new_num_processing = Clamp(10, new_num_processing, 10000);
    new_num_connections = Clamp(1, new_num_connections, 1000);
    
    int new_num_inputs = new_context_length * NUM_CHARS;
    ImGui::Text("Input neurons: %d (%d tokens x %d chars)", new_num_inputs, new_context_length, NUM_CHARS);
    
    if(ImGui::Button("Create New Soup"))
    {
        editor->token_window_size = new_context_length;
        editor->num_input_neurons = new_num_inputs;
        editor->soup = CreateSoup(arena, new_num_inputs, new_num_processing, new_num_connections);
        soup = editor->soup;
        // Reset history
        editor->active_neuron_history.Clear();
        editor->avg_bias_history.Clear();
        editor->firing_rate_history.Clear();
        // Reset chunk tracking
        editor->chunk_accuracy_history.Clear();
        editor->tokens_per_chunk_history.Clear();
        editor->current_chunk_iteration = 0;
        editor->chunk_tokens_processed = 0;
        editor->chunk_accuracy_sum = 0.0f;
        editor->last_chunk_accuracy = 0.0f;
        editor->current_chunk_accuracy = 0.0f;
        editor->total_chunks_processed = 0;
        editor->current_token_index = 0;
        // Reset long-term tracking
        editor->longterm_accuracy_history.Clear();
        editor->longterm_chunk_numbers.Clear();
        editor->longterm_sample_counter = 0;
        editor->longterm_accuracy_sum = 0.0f;
        editor->longterm_samples_in_period = 0;
        // Reset prediction history
        editor->predicted_chars.Clear();
        editor->target_chars.Clear();
        editor->last_predicted_index = -1;
        editor->last_softmax_confidence = 0.0f;
        editor->last_dopamine = 0.0f;
        
        // Set initial input
        SetInputFromContext(soup, editor->training_text, editor->training_text_length,
                           editor->current_token_index, editor->token_window_size);
    }
    
    ImGui::Separator();
    
    // Edit Soup parameters
    ImGui::Text("Soup Parameters");
    ImGui::SliderFloat("Target Rate", &soup->target_rate, 0.0f, 1.0f);
    ImGui::SliderFloat("Bias Learning Rate", &soup->eta_bias, 0.0f, 0.5f);
    ImGui::SliderFloat("Weight Learning Rate", &soup->eta_weight, 0.0f, 0.5f);
    ImGui::SliderFloat("Eligibility Decay", &soup->eligibility_decay, 0.0f, 1.0f);
    ImGui::InputFloat("Learning Rate (Manual)", &soup->eta_weight);
    
    ImGui::Separator();
    
    // Run controls
    ImGui::Text("Run Controls");
    
    // Play/Pause button
    if(editor->soup_playing)
    {
        if(ImGui::Button(ICON_LC_PAUSE " Pause"))
        {
            editor->soup_playing = false;
        }
    }
    else
    {
        if(ImGui::Button(ICON_LC_PLAY " Play"))
        {
            editor->soup_playing = true;
        }
    }
    
    ImGui::SameLine();
    
    // Step button
    if(ImGui::Button(ICON_LC_STEP_FORWARD " Step"))
    {
        UpdateTokens(editor);
    }
    
    // Speed control (steps per frame)
    ImGui::SliderInt("Steps Per Frame", &editor->soup_steps_per_frame, 1, 1000);
    
    // Token training controls
    ImGui::Separator();
    ImGui::Text("Token Training");
    ImGui::Text("Context Length: %d tokens (%d input neurons)", editor->token_window_size, editor->num_input_neurons);
    
    if(editor->training_text)
    {
        ImGui::Text("Current Position: %d / %d", editor->current_token_index, editor->training_text_length);
        
        // Show current sentence
        int sentence_start = editor->current_token_index;
        int sentence_end = editor->current_token_index;
        
        // Find sentence start (previous period, exclamation, or question mark)
        for(int i = editor->current_token_index; i >= 0; i--)
        {
            char c = editor->training_text[i];
            if(c == '.' || c == '!' || c == '?')
            {
                sentence_start = i + 1;
                break;
            }
            if(i == 0) sentence_start = 0;
        }
        
        // Find sentence end (next period, exclamation, or question mark)
        for(int i = editor->current_token_index; i < editor->training_text_length; i++)
        {
            char c = editor->training_text[i];
            if(c == '.' || c == '!' || c == '?')
            {
                sentence_end = i + 1;
                break;
            }
        }
        if(sentence_end == editor->current_token_index)
            sentence_end = editor->training_text_length;
        
        // Display current sentence
        char sentence[512];
        int len = 0;
        for(int i = sentence_start; i < sentence_end && len < 511; i++)
        {
            sentence[len++] = editor->training_text[i];
        }
        sentence[len] = '\0';
        ImGui::Text("Current Sentence: %s", sentence);
        
        if(editor->current_token_index < editor->training_text_length)
        {
            // Show current context
            int start = Max(0, editor->current_token_index - editor->token_window_size);
            int end = Min(editor->training_text_length, editor->current_token_index + 1);
            char context[256];
            len = 0;
            for(int i = start; i < end; i++)
            {
                if(len < 255)
                    context[len++] = editor->training_text[i];
            }
            context[len] = '\0';
            ImGui::Text("Context: %s", context);
            
            if(editor->current_token_index < editor->training_text_length - 1)
            {
                ImGui::Text("Target: '%c'", editor->training_text[editor->current_token_index + 1]);
            }
        }
        ImGui::Text("Last softmax prob for target: %.2f%%", editor->last_accuracy * 100.0f);
        ImGui::Text("Last prediction confidence: %.2f%%", editor->last_softmax_confidence * 100.0f);
        
        // Display prediction history
        if(editor->predicted_chars.size > 0)
        {
            ImGui::Separator();
            ImGui::Text("Prediction History (Last %d tokens):", (int)editor->predicted_chars.size);
            
            // Build predicted string
            char predicted_str[128] = {0};
            char target_str[128] = {0};
            char match_str[128] = {0};
            int display_count = Min((int)editor->predicted_chars.size, 50);
            int start_idx = (int)editor->predicted_chars.size - display_count;
            
            for(int i = 0; i < display_count && i < 127; i++)
            {
                predicted_str[i] = editor->predicted_chars[start_idx + i];
                target_str[i] = editor->target_chars[start_idx + i];
                match_str[i] = (predicted_str[i] == target_str[i]) ? '^' : ' ';
            }
            predicted_str[display_count] = '\0';
            target_str[display_count] = '\0';
            match_str[display_count] = '\0';
            
            // Count correct predictions
            int correct = 0;
            for(int i = 0; i < display_count; i++)
            {
                if(predicted_str[i] == target_str[i]) correct++;
            }
            float hit_rate = (float)correct / (float)display_count * 100.0f;
            
            ImGui::Text("Predicted: %s", predicted_str);
            ImGui::Text("Target:    %s", target_str);
            ImGui::Text("Matches:   %s", match_str);
            ImGui::Text("Hit rate: %d/%d (%.1f%%)", correct, display_count, hit_rate);
        }
        
        // Immediate per-token reward info
        ImGui::Separator();
        ImGui::Text("Immediate Per-Token Reward");
        ImGui::Text("Last dopamine applied: %.3f", editor->last_dopamine);
        ImGui::Text("Baseline (random): %.2f%%", 100.0f / (float)NUM_CHARS);
        
        // Chunk stats (for visualization only)
        ImGui::SliderInt("Stats Window Size", &editor->chunk_size, 10, 1000);
        ImGui::Text("Tokens in window: %d", editor->chunk_tokens_processed);
        ImGui::Text("Window accuracy: %.2f%%", 
            editor->chunk_tokens_processed > 0 
                ? (editor->chunk_accuracy_sum / (float)editor->chunk_tokens_processed) * 100.0f 
                : 0.0f);
        ImGui::Text("Total windows: %d", editor->total_chunks_processed);
    }
    
    // Execute steps if playing
    if(editor->soup_playing)
    {
        for(int i = 0; i < editor->soup_steps_per_frame; i++)
        {
            UpdateTokens(editor);
        }
    }
    
    ImGui::Separator();
    
    // Statistics
    int active_inputs = CountActiveInputs(*soup);
    int active_count = CountActiveNeurons(*soup);
    R32 firing_rate = soup->neurons.size > 0 ? (R32)active_count / (R32)soup->neurons.size : 0.0f;
    
    R32 avg_bias = 0.0f;
    for(Neuron& neuron : soup->neurons)
    {
        avg_bias += neuron.bias;
    }
    if(soup->neurons.size > 0)
    {
        avg_bias /= (R32)soup->neurons.size;
    }
    
    ImGui::Text("Statistics");
    ImGui::Text("Input Neurons: %lld (active: %d)", soup->inputs.size, active_inputs);
    ImGui::Text("Processing Neurons: %lld (active: %d)", soup->neurons.size, active_count);
    ImGui::Text("Firing Rate: %.3f", firing_rate);
    ImGui::Text("Average Bias: %.3f", avg_bias);
    ImGui::Text("Last Dopamine: %.3f", editor->last_dopamine);
    
    // Chunk accuracy graph
    if(editor->chunk_accuracy_history.size > 0)
    {
        ImGui::Separator();
        ImGui::Text("Chunk Accuracy Over Time");
        
        DynamicArray<R32> x_axis_chunks(editor->chunk_accuracy_history.size);
        x_axis_chunks.Fill();
        x_axis_chunks.ApplyIndexed([](int i, R32& val) {val = (R32)i;});
        
        ImPlotFlags chunk_plot_flags = ImPlotFlags_NoBoxSelect | 
                                ImPlotFlags_NoInputs | 
                                ImPlotFlags_NoFrame;
        
        ImPlot::SetNextAxesLimits(0, (int)editor->chunk_accuracy_history.size, 0.0f, 2.0f, ImPlotCond_Always);
        Vec2 chunk_plot_size = V2(-1, 150);
        if(ImPlot::BeginPlot("Chunk Accuracy", ImVec2(chunk_plot_size.x, chunk_plot_size.y), chunk_plot_flags))
        {
            Vec4 accuracy_color = V4(0.2f, 0.8f, 0.2f, 1.0f);
            ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(accuracy_color.x, accuracy_color.y, accuracy_color.z, accuracy_color.w));
            ImPlot::PlotLine("Accuracy", x_axis_chunks.data, editor->chunk_accuracy_history.data, (int)editor->chunk_accuracy_history.size);
            ImPlot::PopStyleColor();
            ImPlot::EndPlot();
        }
        
        // Tokens per chunk graph
        ImPlot::SetNextAxesToFit();
        Vec2 tokens_plot_size = V2(-1, 100);
        if(ImPlot::BeginPlot("Tokens Per Chunk", ImVec2(tokens_plot_size.x, tokens_plot_size.y), chunk_plot_flags))
        {
            Vec4 tokens_color = V4(0.8f, 0.6f, 0.2f, 1.0f);
            ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(tokens_color.x, tokens_color.y, tokens_color.z, tokens_color.w));
            ImPlot::PlotLine("Tokens", x_axis_chunks.data, editor->tokens_per_chunk_history.data, (int)editor->tokens_per_chunk_history.size);
            ImPlot::PopStyleColor();
            ImPlot::EndPlot();
        }
    }
    
    // Long-term accuracy tracking
    ImGui::Separator();
    ImGui::Text("Long-Term Accuracy Tracking");
    ImGui::SliderInt("Sample Every N Chunks", &editor->longterm_sample_rate, 1, 100);
    ImGui::Text("Long-term samples: %lld", editor->longterm_accuracy_history.size);
    ImGui::Text("Next sample in: %d chunks", editor->longterm_sample_rate - editor->longterm_sample_counter);
    
    if(editor->longterm_accuracy_history.size > 1)
    {
        ImPlotFlags longterm_plot_flags = ImPlotFlags_NoBoxSelect | 
                                ImPlotFlags_NoFrame;
        
        // Use chunk numbers as x-axis for proper scaling
        ImPlot::SetNextAxesToFit();
        Vec2 longterm_plot_size = V2(-1, 200);
        if(ImPlot::BeginPlot("Long-Term Accuracy (All Time)", ImVec2(longterm_plot_size.x, longterm_plot_size.y), longterm_plot_flags))
        {
            ImPlot::SetupAxes("Chunk #", "Accuracy");
            Vec4 longterm_color = V4(0.2f, 0.6f, 1.0f, 1.0f);
            ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(longterm_color.x, longterm_color.y, longterm_color.z, longterm_color.w));
            ImPlot::PlotLine("Avg Accuracy", editor->longterm_chunk_numbers.data, editor->longterm_accuracy_history.data, (int)editor->longterm_accuracy_history.size);
            ImPlot::PopStyleColor();
            ImPlot::EndPlot();
        }
        
        // Show min/max/current long-term accuracy
        float min_acc = editor->longterm_accuracy_history[0];
        float max_acc = editor->longterm_accuracy_history[0];
        for(int i = 1; i < editor->longterm_accuracy_history.size; i++)
        {
            if(editor->longterm_accuracy_history[i] < min_acc) min_acc = editor->longterm_accuracy_history[i];
            if(editor->longterm_accuracy_history[i] > max_acc) max_acc = editor->longterm_accuracy_history[i];
        }
        float latest_acc = editor->longterm_accuracy_history[editor->longterm_accuracy_history.size - 1];
        ImGui::Text("Min: %.2f%% | Max: %.2f%% | Latest: %.2f%%", min_acc * 100.0f, max_acc * 100.0f, latest_acc * 100.0f);
        
        // Show trend (compare first 10% vs last 10%)
        if(editor->longterm_accuracy_history.size >= 10)
        {
            int window = Max(1, (int)editor->longterm_accuracy_history.size / 10);
            float early_avg = 0.0f;
            float late_avg = 0.0f;
            for(int i = 0; i < window; i++)
            {
                early_avg += editor->longterm_accuracy_history[i];
                late_avg += editor->longterm_accuracy_history[editor->longterm_accuracy_history.size - 1 - i];
            }
            early_avg /= (float)window;
            late_avg /= (float)window;
            float change = late_avg - early_avg;
            const char* trend = change > 0.01f ? "IMPROVING" : (change < -0.01f ? "DECLINING" : "STABLE");
            ImGui::Text("Trend: %s (%.2f%% -> %.2f%%)", trend, early_avg * 100.0f, late_avg * 100.0f);
        }
    }
    
    // Track history
    editor->history_track_counter--;
    if(editor->history_track_counter <= 0)
    {
        editor->history_track_counter = editor->track_history_per;
        
        // Shift arrays and add new data
        const int max_history = 200;
        if(editor->active_neuron_history.size >= max_history)
        {
            editor->active_neuron_history.Shift(-1);
            editor->avg_bias_history.Shift(-1);
            editor->firing_rate_history.Shift(-1);
        }
        else
        {
            editor->active_neuron_history.PushBack((R32)active_count);
            editor->avg_bias_history.PushBack(avg_bias);
            editor->firing_rate_history.PushBack(firing_rate);
        }
        
        // Update last values
        if(editor->active_neuron_history.size > 0)
        {
            editor->active_neuron_history[editor->active_neuron_history.size - 1] = (R32)active_count;
            editor->avg_bias_history[editor->avg_bias_history.size - 1] = avg_bias;
            editor->firing_rate_history[editor->firing_rate_history.size - 1] = firing_rate;
        }
    }
    else if(editor->active_neuron_history.size > 0)
    {
        // Update last value even if not tracking new point
        editor->active_neuron_history[editor->active_neuron_history.size - 1] = (R32)active_count;
        editor->avg_bias_history[editor->avg_bias_history.size - 1] = avg_bias;
        editor->firing_rate_history[editor->firing_rate_history.size - 1] = firing_rate;
    }
    
    ImGui::Separator();
    
    // Visualizations
    ImPlotFlags plot_flags = ImPlotFlags_NoBoxSelect | 
                            ImPlotFlags_NoInputs | 
                            ImPlotFlags_NoFrame | 
                            ImPlotFlags_NoLegend;
    
    // Active neurons over time
    if(editor->active_neuron_history.size > 0)
    {
        DynamicArray<R32> x_axis(editor->active_neuron_history.size);
        x_axis.Fill();
        x_axis.ApplyIndexed([](int i, R32& val) {val = (R32)i;});
        
        ImPlot::SetNextAxesLimits(0, (int)editor->active_neuron_history.size, 0, (R32)soup->neurons.size, ImPlotCond_Always);
        Vec2 plot_size = V2(-1, 150);
        if(ImPlot::BeginPlot("Active Neurons Over Time", ImVec2(plot_size.x, plot_size.y), plot_flags))
        {
            ImPlot::PlotLine("Active", x_axis.data, editor->active_neuron_history.data, (int)editor->active_neuron_history.size);
            ImPlot::EndPlot();
        }
    }
    
    // Firing rate over time
    if(editor->firing_rate_history.size > 0)
    {
        DynamicArray<R32> x_axis(editor->firing_rate_history.size);
        x_axis.Fill();
        x_axis.ApplyIndexed([](int i, R32& val) {val = (R32)i;});
        
        ImPlot::SetNextAxesLimits(0, (int)editor->firing_rate_history.size, 0.0f, 1.0f, ImPlotCond_Always);
        Vec2 plot_size2 = V2(-1, 150);
        if(ImPlot::BeginPlot("Firing Rate Over Time", ImVec2(plot_size2.x, plot_size2.y), plot_flags))
        {
            Vec4 color1 = V4(0.3f, 1.0f, 0.3f, 1.0f);
            ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(color1.x, color1.y, color1.z, color1.w));
            ImPlot::PlotLine("Firing Rate", x_axis.data, editor->firing_rate_history.data, (int)editor->firing_rate_history.size);
            ImPlot::PopStyleColor();
            
            // Draw target rate line
            R32 target_line[2] = {soup->target_rate, soup->target_rate};
            R32 target_x[2] = {0.0f, (R32)editor->firing_rate_history.size};
            Vec4 color2 = V4(1.0f, 0.3f, 0.3f, 0.5f);
            ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(color2.x, color2.y, color2.z, color2.w));
            ImPlot::PlotLine("Target", target_x, target_line, 2);
            ImPlot::PopStyleColor();
            
            ImPlot::EndPlot();
        }
    }
    
    // Average bias over time
    if(editor->avg_bias_history.size > 0)
    {
        DynamicArray<R32> x_axis(editor->avg_bias_history.size);
        x_axis.Fill();
        x_axis.ApplyIndexed([](int i, R32& val) {val = (R32)i;});
        
        ImPlot::SetNextAxesToFit();
        Vec2 plot_size3 = V2(-1, 150);
        if(ImPlot::BeginPlot("Average Bias Over Time", ImVec2(plot_size3.x, plot_size3.y), plot_flags))
        {
            Vec4 color3 = V4(0.3f, 0.3f, 1.0f, 1.0f);
            ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(color3.x, color3.y, color3.z, color3.w));
            ImPlot::PlotLine("Bias", x_axis.data, editor->avg_bias_history.data, (int)editor->avg_bias_history.size);
            ImPlot::PopStyleColor();
            ImPlot::EndPlot();
        }
    }
    
    // Neuron states visualization (sample of neurons)
    const int sample_size = Min(100, (int)soup->neurons.size);
    if(sample_size > 0)
    {
        DynamicArray<R32> neuron_states(sample_size);
        DynamicArray<R32> neuron_indices(sample_size);
        
        for(int i = 0; i < sample_size; i++)
        {
            int idx = (int)((I64)i * soup->neurons.size / sample_size);
            neuron_states.PushBack((R32)soup->neurons[idx].state);
            neuron_indices.PushBack((R32)i);
        }
        
        ImPlot::SetNextAxesLimits(-1, sample_size, -0.5f, 1.5f, ImPlotCond_Always);
        Vec2 plot_size4 = V2(-1, 150);
        if(ImPlot::BeginPlot("Neuron States (Sample)", ImVec2(plot_size4.x, plot_size4.y), plot_flags))
        {
            Vec4 color4 = V4(0.3f, 0.8f, 0.3f, 0.6f);
            ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4(color4.x, color4.y, color4.z, color4.w));
            ImPlot::PlotBars("States", neuron_indices.data, neuron_states.data, sample_size, 0.5f);
            ImPlot::PopStyleColor();
            ImPlot::EndPlot();
        }
    }
    
    ImGui::End();
}

int
UpdateEditorScreen(EditorScreen* editor, Window* window)
{
    InputHandler* input = &window->input;
    Camera2D* cam = &editor->cam;
    TiltedRenderer* renderer = editor->renderer;

    Agent* agent = editor->agent;

    // Do ui 
    ImGui::BeginMainMenuBar();
    if(ImGui::BeginMenu("Window"))
    {
        if(ImGui::MenuItem("Back to simulation" ICON_LC_ARROW_BIG_LEFT))
        {
            global_settings.current_phase = GamePhase_SimulationScreen;
        }
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();

    // EditCreatureWindowHidden is not called - moved to separate function
    
    // Soup editor window
    EditSoupWindow(editor);

    R32 tilt_speed = -0.01f;
    if(IsKeyDown(input, InputAction_W))
    {
        renderer->cam.angle += tilt_speed;
    }
    if(IsKeyDown(input, InputAction_S))
    {
        renderer->cam.angle -= tilt_speed;
    }

    renderer->cam.angle = Clamp(-PI_R32/2.0f+0.001f, renderer->cam.angle, 0.0f);

    UpdateTiltedCamera(&editor->renderer->cam, window->width, window->height);

    ImGuiIO& imgui_io = ImGui::GetIO();
    if(!imgui_io.WantCaptureMouse)
    {
        UpdateTiltedCameraScrollInput(&editor->renderer->cam, input);
        UpdateTiltedCameraDragInput(&editor->renderer->cam, input);
    }

    R32 grayscale = 0.6f;
    U32 platform_color = Vec4ToColor(grayscale, grayscale, grayscale, 1.0f);
    R32 shade = 0.6f;
    U32 platform_shade = Vec4ToColor(grayscale*shade, grayscale*shade, grayscale*shade, 1.0f);
    R32 platform_r = walk_radius+25.0f;
    R32 platform_height = 35.0f;
    R32 pedestal_r = platform_r*0.77f;
    R32 pedestal_height = 200.0f;
    AtlasRegion* square = renderer->renderer->square;

    RenderZCircle(renderer, V3(0,0,-pedestal_height), pedestal_r, platform_shade);
    RenderYRect(renderer, V3(0,0,-pedestal_height/2.0f), V2(pedestal_r*2, pedestal_height), square, platform_shade);
    RenderZCircle(renderer, V3(0,0,-platform_height), platform_r, platform_shade);
    RenderYRect(renderer, V3(0,0,-platform_height/2.0f), V2(platform_r*2, platform_height), square, platform_shade);
    RenderZCircle(renderer, V3(0,0,0), platform_r, platform_color);

    // Draw mouse position
    Vec3 mouse = TiltedMouseToWorld(&renderer->cam, input, window->width, window->height);
    R32 cursor_line_width = 0.1f;
    R32 cursor_radius = 1.0f+sinf(editor->time*2)*0.25f;
    RenderZLineCircle(renderer, mouse, cursor_radius, cursor_line_width, Color_Cyan);

    // Make the agent walk
    Vec3 direction = V3(0,0,0);
    direction.xy = V2Polar(agent->orientation, 1.0f);
    agent->pos += direction.xy * speed;
    agent->orientation += turn_speed;
    UpdateAgentSkeleton(agent);
    RenderAgent(renderer, agent);

    Vec3 agent_pos = V3(agent->pos.x, agent->pos.y, 0);
    
    // Debug rendering
    RenderCircle(renderer, agent_pos, 0.1f, Color_Red);
    for(Leg& leg : agent->legs)
    {
        Vec3 leg_pos = XForm(agent_pos, direction.xy, leg.target_offset);
        RenderCircle(renderer, leg_pos, 0.1f, Color_Green);
        RenderZLineCircle(renderer, leg_pos, leg.r, 0.03f, Color_Green);
    }

    // Render entire thing 
    Render(renderer->renderer, cam, window->width, window->height);

    editor->time += 1.0f/60.0f;

    return 0;
}

void
InitEditorScreen(EditorScreen* editor)
{
    // Might be a bit much memory for the editor.
    MemoryArena* arena = editor->editor_arena = CreateMemoryArena(MegaBytes(256));
    editor->time = 0.0f;
    editor->cam.pos = V2(0,0);
    editor->cam.scale = 1;
    editor->renderer = CreateTiltedRenderer(arena);
    editor->renderer->cam.angle = -PI_R32/4.0f;
    editor->renderer->cam.scale = 6.0;
    editor->agent = PushNewStruct(arena, Agent);
    editor->agent->skeleton = CreateSkeleton(arena, max_joints, max_joints*2);
    editor->agent->pos = V2(0, -walk_radius);

    PhenoType* pheno = editor->agent->phenotype = CreatePhenotype(arena, 12);
    InitRandomPhenotype(pheno);
    InitAgentSkeleton(arena, editor->agent);

    // Tracking arrays are initialized via constructor
    editor->track_history_per = 1;
    editor->history_track_counter = 0;
    
    // Initialize token training with default context length of 10
    editor->training_text = TRAINING_TEXT;
    editor->training_text_length = (int)strlen(TRAINING_TEXT);
    editor->current_token_index = 0;
    editor->token_window_size = 10;  // Default context length
    editor->num_chars = NUM_CHARS;
    editor->num_actuator_neurons = 10;
    editor->num_output_neurons = NUM_CHARS;
    editor->num_input_neurons = editor->token_window_size * NUM_CHARS;
    editor->steps_since_actuator = 0;
    editor->waiting_for_actuator = false;
    editor->last_accuracy = 0.0f;
    editor->last_predicted_index = -1;
    editor->last_softmax_confidence = 0.0f;
    editor->last_dopamine = 0.0f;
    
    // Create soup with separate input layer
    // CreateSoup(arena, num_inputs, num_processing, num_connections)
    editor->soup = CreateSoup(arena, editor->num_input_neurons, 1000, 10);
    
    // Initialize chunk tracking (for visualization)
    editor->chunk_size = 100;
    editor->current_chunk_iteration = 0;
    editor->chunk_tokens_processed = 0;
    editor->chunk_accuracy_sum = 0.0f;
    editor->last_chunk_accuracy = 0.0f;
    editor->current_chunk_accuracy = 0.0f;
    editor->dopamine_released_this_chunk = false;
    editor->total_chunks_processed = 0;
    
    // Initialize long-term tracking
    editor->longterm_sample_rate = 10;
    editor->longterm_sample_counter = 0;
    editor->longterm_accuracy_sum = 0.0f;
    editor->longterm_samples_in_period = 0;
    
    // Set initial input using the separate input layer
    if(editor->soup && editor->training_text)
    {
        SetInputFromContext(editor->soup, editor->training_text, editor->training_text_length,
                           editor->current_token_index, editor->token_window_size);
    }
}
