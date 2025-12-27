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

// Helper: Set input layer from context
void SetInputFromContext(Neuron* inputs, const char* text, int text_length, int current_index, int window_size, int num_chars)
{
    int input_idx = 0;
    for(int token = 0; token < window_size; token++)
    {
        int text_idx = current_index - window_size + 1 + token;

        // Wrap around properly (modulo)
        while(text_idx < 0) text_idx += text_length;
        while(text_idx >= text_length) text_idx -= text_length;

        char c = text[text_idx];
        int char_idx = CharToIndex(c);

        for(int i = 0; i < num_chars && input_idx < window_size * num_chars; i++)
        {
            inputs[input_idx].state = (i == char_idx) ? 1 : 0;
            input_idx++;
        }
    }
}

void UpdateTokens(EditorScreen* editor)
{
    Soup* soup = editor->soup;
    if(!soup) return;

    // Get target character (next character in sequence)
    int next_index = editor->current_token_index + 1;
    if(next_index >= editor->training_text_length)
        next_index = 0;  // Wrap around

    char target_char = editor->training_text[next_index];
    int target_index = CharToIndex(target_char);

    // Forward pass
    ForwardSoup(*soup);

    // Check next_token signal (outputs[31] vs outputs[32]) using softmax
    R32 next_act = soup->outputs[31].activation;
    R32 not_next_act = soup->outputs[32].activation;
    R32 max_act = Max(next_act, not_next_act);
    R32 next_exp = expf(next_act - max_act);
    R32 not_next_exp = expf(not_next_act - max_act);
    R32 sum_exp = next_exp + not_next_exp;
    R32 next_prob = next_exp / sum_exp;

    // Predict character (argmax over outputs[0-30])
    R32 char_max_act = -1e30f;
    int predicted_char_idx = 0;
    for(int i = 0; i < 31; i++)
    {
        if(soup->outputs[i].activation > char_max_act)
        {
            char_max_act = soup->outputs[i].activation;
            predicted_char_idx = i;
        }
    }

    // Compute softmax probabilities for both target and predicted
    R32 char_sum_exp = 0.0f;
    R32 char_target_exp = 0.0f;
    R32 char_predicted_exp = 0.0f;
    for(int i = 0; i < 31; i++)
    {
        R32 exp_val = expf(soup->outputs[i].activation - char_max_act);
        char_sum_exp += exp_val;
        if(i == target_index)
            char_target_exp = exp_val;
        if(i == predicted_char_idx)
            char_predicted_exp = exp_val;
    }
    R32 target_prob = char_target_exp / char_sum_exp;
    R32 char_confidence = char_predicted_exp / char_sum_exp;

    editor->last_predicted_index = predicted_char_idx;
    editor->last_softmax_confidence = char_confidence;
    editor->last_accuracy = target_prob;  // Probability assigned to the correct target

    // Network decides to predict when next_token has higher activation than not_next_token
    bool network_wants_predict = (next_prob > 0.5f);
    bool correct = (predicted_char_idx == target_index);
    
    // Track steps since last prediction
    editor->steps_since_actuator++;
    
    // Inactivity thresholds
    const int PUNISH_THRESHOLD = 30;   // Punish for not predicting after this many steps
    const int FORCE_THRESHOLD = 50;    // Force prediction after this many steps
    
    bool too_long_without_predict = (editor->steps_since_actuator > PUNISH_THRESHOLD);
    bool force_predict = (editor->steps_since_actuator >= FORCE_THRESHOLD);
    
    // Determine if we should do a training step
    bool should_train = network_wants_predict || too_long_without_predict;
    bool should_advance = network_wants_predict || force_predict;
    
    if(should_train)
    {
        // Build correct pattern for character prediction
        int correct_pattern[33];
        for(int i = 0; i < 31; i++)
            correct_pattern[i] = (i == target_index) ? 1 : 0;
        
        // Determine actuator target based on situation:
        // - Network predicted correctly: actuator SHOULD have fired
        // - Network predicted wrongly: actuator should NOT have fired
        // - Network stayed silent too long: actuator SHOULD have fired (punish inactivity)
        bool actuator_should_fire;
        if(network_wants_predict)
        {
            // Network chose to predict - was it right?
            actuator_should_fire = correct;
        }
        else
        {
            // Network didn't predict but we're training due to inactivity
            // Punish: actuator SHOULD have fired (teach it to be more active)
            actuator_should_fire = true;
        }
        
        if(actuator_should_fire)
        {
            correct_pattern[31] = 1;  // next_token ON
            correct_pattern[32] = 0;  // not_next_token OFF
        }
        else
        {
            correct_pattern[31] = 0;  // next_token OFF
            correct_pattern[32] = 1;  // not_next_token ON
        }

        // Set output states from network's actual predictions BEFORE learning
        // For character outputs (0-30): use argmax (one-hot)
        for(int i = 0; i < 31; i++)
            soup->outputs[i].state = (i == predicted_char_idx) ? 1 : 0;
        // For next_token signals (31-32): what network actually output
        soup->outputs[31].state = network_wants_predict ? 1 : 0;
        soup->outputs[32].state = network_wants_predict ? 0 : 1;

        // Determine if this counts as "correct" for learning purposes
        bool learning_correct = network_wants_predict && correct;
        
        // Learn (ApplyOutputLearning uses states vs target to compute error)
        UpdateEligibility(*soup);
        ApplyOutputLearning(*soup, correct_pattern, learning_correct);

        // Dopamine: positive for correct predictions, negative otherwise
        R32 dopamine;
        if(network_wants_predict && correct)
            dopamine = 1.0f;      // Good prediction!
        else if(network_wants_predict && !correct)
            dopamine = -0.5f;     // Bad prediction - should have waited
        else
            dopamine = -0.3f;     // Too inactive - should have predicted
        
        ApplyHebbian(*soup, dopamine);

        // NOW force outputs to correct pattern for next timestep
        for(int i = 0; i < 33; i++)
            soup->outputs[i].state = correct_pattern[i];

        editor->last_dopamine = dopamine;

        // Track accuracy only for actual predictions (not forced)
        if(network_wants_predict)
        {
            editor->chunk_accuracy_sum += correct ? 1.0f : 0.0f;
            editor->chunk_tokens_processed++;

            // Prediction history
            char predicted_char = IndexToChar(predicted_char_idx);
            if(editor->predicted_chars.size >= editor->prediction_history_size)
            {
                editor->predicted_chars.Shift(-1);
                editor->predicted_chars.size--;
                editor->target_chars.Shift(-1);
                editor->target_chars.size--;
            }
            editor->predicted_chars.PushBack(predicted_char);
            editor->target_chars.PushBack(target_char);
        }
    }
    else
    {
        editor->last_dopamine = 0.0f;
    }
    
    // Advance token when network predicts OR forced after timeout
    if(should_advance)
    {
        editor->steps_since_actuator = 0;  // Reset counter
        
        editor->current_token_index++;
        if(editor->current_token_index >= editor->training_text_length)
            editor->current_token_index = 0;

        SetInputFromContext(soup->inputs.data, editor->training_text, editor->training_text_length,
                           editor->current_token_index, editor->token_window_size, NUM_CHARS);
    }

    ApplyHomeostasis(*soup);
    StorePrevStates(*soup);

    // Chunk statistics
    editor->current_chunk_iteration++;
    if(editor->current_chunk_iteration >= editor->chunk_size)
    {
        if(editor->chunk_tokens_processed > 0)
            editor->current_chunk_accuracy = editor->chunk_accuracy_sum / (float)editor->chunk_tokens_processed;
        else
            editor->current_chunk_accuracy = 0.0f;

        const int max_history = 200;
        if(editor->chunk_accuracy_history.size >= max_history)
        {
            editor->chunk_accuracy_history.Shift(-1);
            editor->chunk_accuracy_history.size--;
            editor->tokens_per_chunk_history.Shift(-1);
            editor->tokens_per_chunk_history.size--;
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

        editor->current_chunk_iteration = 0;
        editor->chunk_tokens_processed = 0;
        editor->chunk_accuracy_sum = 0.0f;
    }
}

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

void EditChatWindow(EditorScreen* editor)
{
    if(!editor->soup) return;

    ImGui::Begin("Chat with Model", &editor->chat_show_window);

    ImGui::Text("Talk to the neural network!");
    ImGui::Text("Enter a prompt and the network will generate text.");

    ImGui::Separator();

    // Input field
    ImGui::Text("Your prompt:");
    ImGui::InputText("##prompt", editor->chat_input_buffer, sizeof(editor->chat_input_buffer));

    ImGui::SliderInt("Tokens to generate", &editor->chat_tokens_to_generate, 1, 200);

    if(ImGui::Button("Generate") && strlen(editor->chat_input_buffer) > 0)
    {
        // Clear previous output
        editor->chat_output.Clear();

        // Set input from the prompt
        const char* prompt = editor->chat_input_buffer;
        int prompt_len = (int)strlen(prompt);

        // Add prompt to output
        for(int i = 0; i < prompt_len; i++)
            editor->chat_output.PushBack(prompt[i]);

        // Generate tokens
        for(int gen = 0; gen < editor->chat_tokens_to_generate; gen++)
        {
            // Set input from last N characters of current output
            int context_start = Max(0, (int)editor->chat_output.size - editor->token_window_size);
            int context_len = (int)editor->chat_output.size - context_start;

            // Set input neurons from context
            int input_idx = 0;
            for(int token = 0; token < editor->token_window_size; token++)
            {
                char c = ' ';
                int char_offset = context_start + token;
                if(char_offset < editor->chat_output.size)
                    c = editor->chat_output[char_offset];
                else
                    c = ' ';  // Pad with spaces if context too short

                int char_idx = CharToIndex(c);
                for(int i = 0; i < NUM_CHARS && input_idx < editor->soup->inputs.size; i++)
                {
                    editor->soup->inputs[input_idx].state = (i == char_idx) ? 1 : 0;
                    input_idx++;
                }
            }

            // Forward pass
            ForwardSoup(*editor->soup);

            // Get prediction (argmax over outputs[0-30])
            R32 char_max_act = -1e30f;
            int predicted_char_idx = 0;
            for(int i = 0; i < 31; i++)
            {
                if(editor->soup->outputs[i].activation > char_max_act)
                {
                    char_max_act = editor->soup->outputs[i].activation;
                    predicted_char_idx = i;
                }
            }

            char predicted_char = IndexToChar(predicted_char_idx);
            editor->chat_output.PushBack(predicted_char);

            // Update states for next iteration (no learning in chat mode)
            for(int i = 0; i < 33; i++)
                editor->soup->outputs[i].state = (i == predicted_char_idx && i < 31) ? 1 : 0;
            StorePrevStates(*editor->soup);
        }
    }

    ImGui::SameLine();
    if(ImGui::Button("Clear"))
    {
        editor->chat_output.Clear();
        editor->chat_input_buffer[0] = '\0';
    }

    ImGui::Separator();

    // Display output
    if(editor->chat_output.size > 0)
    {
        ImGui::Text("Generated text:");

        // Build output string
        char output_str[2048] = {0};
        int copy_len = Min((int)editor->chat_output.size, 2047);
        for(int i = 0; i < copy_len; i++)
            output_str[i] = editor->chat_output[i];
        output_str[copy_len] = '\0';

        ImGui::TextWrapped("%s", output_str);
    }

    ImGui::End();
}

void EditSoupWindow(EditorScreen* editor)
{
    if(!editor->soup) return;
    
    Soup* soup = editor->soup;
    MemoryArena* arena = editor->editor_arena;
    
    ImGui::Begin("Soup Editor");
    
    // Create new Soup
    static int new_num_processing = 1000;
    static int new_num_connections = 100;  // Default to 100 for better connectivity
    static int new_context_length = 10;
    ImGui::Text("Create New Soup");
    ImGui::InputInt("Context Length (tokens)", &new_context_length);
    ImGui::InputInt("Processing Neurons", &new_num_processing);
    ImGui::InputInt("Hidden Connections per Neuron", &new_num_connections);
    new_context_length = Clamp(1, new_context_length, 20);
    new_num_processing = Clamp(10, new_num_processing, 10000);
    new_num_connections = Clamp(1, new_num_connections, 1000);

    int new_num_inputs = new_context_length * NUM_CHARS;
    ImGui::Text("Input neurons: %d (%d tokens x %d chars)", new_num_inputs, new_context_length, NUM_CHARS);

    if(ImGui::Button("Create New Soup"))
    {
        editor->token_window_size = new_context_length;
        editor->num_input_neurons = new_num_inputs;
        editor->num_output_neurons = 33;  // 31 char + 2 next_token

        // Create Soup: Input -> Hidden -> Output (char + next_token)
        int hidden_connections = new_num_connections;
        int output_connections = new_num_connections * 5;  // Output much more connected (needs to integrate from hidden + input)
        editor->soup = CreateSoup(arena, new_num_inputs, new_num_processing, 33,
                                  hidden_connections, output_connections);

        // Reset history
        editor->active_neuron_history.Clear();
        editor->avg_bias_history.Clear();
        editor->firing_rate_history.Clear();
        editor->chunk_accuracy_history.Clear();
        editor->tokens_per_chunk_history.Clear();
        editor->current_chunk_iteration = 0;
        editor->chunk_tokens_processed = 0;
        editor->chunk_accuracy_sum = 0.0f;
        editor->last_chunk_accuracy = 0.0f;
        editor->current_chunk_accuracy = 0.0f;
        editor->total_chunks_processed = 0;
        editor->current_token_index = 0;
        editor->longterm_accuracy_history.Clear();
        editor->longterm_chunk_numbers.Clear();
        editor->longterm_sample_counter = 0;
        editor->longterm_accuracy_sum = 0.0f;
        editor->longterm_samples_in_period = 0;
        editor->predicted_chars.Clear();
        editor->target_chars.Clear();
        editor->last_predicted_index = -1;
        editor->last_softmax_confidence = 0.0f;
        editor->last_dopamine = 0.0f;

        // Set initial input
        SetInputFromContext(editor->soup->inputs.data, editor->training_text, editor->training_text_length,
                           editor->current_token_index, editor->token_window_size, NUM_CHARS);
    }
    
    ImGui::Separator();

    // Edit Soup parameters
    ImGui::Text("Soup Parameters");
    ImGui::SliderFloat("Hidden Target Rate", &soup->hidden_target_rate, 0.0f, 0.5f);
    ImGui::SliderFloat("Bias Learning Rate", &soup->eta_bias, 0.0f, 0.01f);
    ImGui::SliderFloat("Hebbian Learning (Hidden)", &soup->eta_hidden_hebbian, 0.0f, 0.2f);
    ImGui::SliderFloat("Perceptron Learning (Output)", &soup->eta_output_perceptron, 0.0f, 1.0f);
    ImGui::SliderFloat("Eligibility Decay", &soup->eligibility_decay, 0.0f, 1.0f);

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

    ImGui::SameLine();

    // Chat button
    if(ImGui::Button(ICON_LC_MESSAGE_CIRCLE " Chat"))
    {
        editor->chat_show_window = !editor->chat_show_window;
    }

    // Speed control (steps per frame)
    ImGui::SliderInt("Steps Per Frame", &editor->soup_steps_per_frame, 1, 1000);
    
    // Token training controls
    ImGui::Separator();
    ImGui::Text("Token Training");
    ImGui::Text("Training text length: %d characters", editor->training_text_length);
    ImGui::Text("Training text: \"%s\"", TRAINING_TEXT);
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
        }

        // Display current position and next target
        ImGui::Text("Current index: %d / %d", editor->current_token_index, editor->training_text_length);
        if(editor->current_token_index < editor->training_text_length)
        {
            ImGui::Text("Current char: '%c'", editor->training_text[editor->current_token_index]);
        }

        int next_idx = editor->current_token_index + 1;
        if(next_idx >= editor->training_text_length) next_idx = 0;
        ImGui::Text("Next target: '%c' (index %d)", editor->training_text[next_idx], next_idx);

        ImGui::Text("Last softmax prob for target: %.2f%%", editor->last_accuracy * 100.0f);
        ImGui::Text("Last prediction confidence: %.2f%%", editor->last_softmax_confidence * 100.0f);
        
        // Display prediction history as text
        if(editor->predicted_chars.size > 0)
        {
            ImGui::Separator();

            int total_count = (int)editor->predicted_chars.size;
            int display_count = Min(total_count, 50);
            int start_idx = total_count - display_count;

            // Count correct predictions
            int correct = 0;
            for(int i = 0; i < display_count; i++)
            {
                char pred = editor->predicted_chars[start_idx + i];
                char targ = editor->target_chars[start_idx + i];
                if(pred == targ) correct++;
            }
            float hit_rate = (float)correct / (float)display_count * 100.0f;

            ImGui::Text("Prediction History (last %d): %d correct (%.1f%%) - Random: 3.2%%",
                        display_count, correct, hit_rate);

            // Build strings for predicted and target
            char predicted_str[128] = {0};
            char target_str[128] = {0};
            char match_str[128] = {0};
            for(int i = 0; i < display_count && i < 127; i++)
            {
                predicted_str[i] = editor->predicted_chars[start_idx + i];
                target_str[i] = editor->target_chars[start_idx + i];
                match_str[i] = (predicted_str[i] == target_str[i]) ? '^' : ' ';
            }
            predicted_str[display_count] = '\0';
            target_str[display_count] = '\0';
            match_str[display_count] = '\0';

            ImGui::Text("Predicted: %s", predicted_str);
            ImGui::Text("Target:    %s", target_str);
            ImGui::Text("Match:     %s", match_str);
            
            // Debug: Show current output activations
            if(ImGui::TreeNode("Output Activations (Debug)"))
            {
                // Find top 5 character activations
                struct CharAct { int idx; R32 act; };
                CharAct top5[5] = {{0, -1e30f}, {0, -1e30f}, {0, -1e30f}, {0, -1e30f}, {0, -1e30f}};
                
                for(int i = 0; i < 31; i++)
                {
                    R32 act = soup->outputs[i].activation;
                    // Insert into top5 if larger
                    for(int j = 0; j < 5; j++)
                    {
                        if(act > top5[j].act)
                        {
                            // Shift down
                            for(int k = 4; k > j; k--)
                                top5[k] = top5[k-1];
                            top5[j].idx = i;
                            top5[j].act = act;
                            break;
                        }
                    }
                }
                
                ImGui::Text("Top 5 character activations:");
                for(int i = 0; i < 5; i++)
                {
                    ImGui::Text("  %d. '%c' (idx %d): %.3f", 
                               i+1, IndexToChar(top5[i].idx), top5[i].idx, top5[i].act);
                }
                
                // Show actuator activations
                ImGui::Separator();
                ImGui::Text("Actuator neurons:");
                ImGui::Text("  next_token[31]: %.3f", soup->outputs[31].activation);
                ImGui::Text("  not_next[32]:   %.3f", soup->outputs[32].activation);
                
                // Show weight statistics
                R32 min_w = 1e30f, max_w = -1e30f, sum_w = 0.0f;
                int count_w = 0;
                for(Neuron& out : soup->outputs)
                {
                    for(Synapse& syn : out.connections)
                    {
                        if(syn.weight < min_w) min_w = syn.weight;
                        if(syn.weight > max_w) max_w = syn.weight;
                        sum_w += syn.weight;
                        count_w++;
                    }
                }
                ImGui::Separator();
                ImGui::Text("Output layer weights:");
                ImGui::Text("  Min: %.3f, Max: %.3f, Avg: %.3f", 
                           min_w, max_w, count_w > 0 ? sum_w / count_w : 0.0f);
                
                // Check if weights have changed from init
                bool weights_near_init = (max_w < 1.0f && min_w > -1.0f);
                if(weights_near_init)
                {
                    ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), 
                        "Weights still near initialization range!");
                }
                
                ImGui::TreePop();
            }
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

    // Statistics (for both V1 and V2)
    int active_inputs = 0;
    int active_count = 0;
    R32 firing_rate = 0.0f;

    active_inputs = CountActiveInputs(*soup);
    active_count = CountActiveHidden(*soup);
    firing_rate = soup->hidden.size > 0 ? (R32)active_count / (R32)soup->hidden.size : 0.0f;

    ImGui::Text("Statistics");
    ImGui::Text("Input Neurons: %lld (active: %d)", soup->inputs.size, active_inputs);
    ImGui::Text("Hidden Neurons: %lld (active: %d)", soup->hidden.size, active_count);
    ImGui::Text("Output Neurons: %lld (31 char + 2 next_token)", soup->outputs.size);
    ImGui::Text("Firing Rate: %.3f", firing_rate);

    // Compute average hidden bias
    R32 avg_bias = 0.0f;
    for(Neuron& n : soup->hidden)
        avg_bias += n.bias;
    if(soup->hidden.size > 0)
        avg_bias /= (R32)soup->hidden.size;
    ImGui::Text("Avg Hidden Bias: %.3f", avg_bias);
    
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
            editor->active_neuron_history.size--;
            editor->avg_bias_history.Shift(-1);
            editor->avg_bias_history.size--;
            editor->firing_rate_history.Shift(-1);
            editor->firing_rate_history.size--;
        }
        // Always push the new values
        editor->active_neuron_history.PushBack((R32)active_count);
        editor->avg_bias_history.PushBack(avg_bias);
        editor->firing_rate_history.PushBack(firing_rate);
        
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
        
        ImPlot::SetNextAxesLimits(0, (int)editor->active_neuron_history.size, 0, (R32)soup->hidden.size, ImPlotCond_Always);
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
            R32 target_line[2] = {soup->hidden_target_rate, soup->hidden_target_rate};
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
    const int sample_size = Min(100, (int)soup->hidden.size);
    if(sample_size > 0)
    {
        DynamicArray<R32> neuron_states(sample_size);
        DynamicArray<R32> neuron_indices(sample_size);

        for(int i = 0; i < sample_size; i++)
        {
            int idx = (int)((I64)i * soup->hidden.size / sample_size);
            neuron_states.PushBack((R32)soup->hidden[idx].state);
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

    // Simple test window (minimal learning test)

    // Soup editor window
    EditSoupWindow(editor);

    // Chat window (only shown when enabled)
    if(editor->chat_show_window)
        EditChatWindow(editor);

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
    
    // Create soup: Input -> Hidden -> Output (33 = 31 char + 2 next_token)
    int hidden_connections = 100;  // Each hidden neuron sees ~32% of 310 inputs
    int output_connections = 500;  // Each output neuron sees ~38% of 1310 sources (inputs + hidden)
    editor->soup = CreateSoup(arena, editor->num_input_neurons, 1000, 33,
                             hidden_connections, output_connections);
    
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
    
    // Set initial input
    if(editor->soup && editor->training_text)
    {
        SetInputFromContext(editor->soup->inputs.data, editor->training_text, editor->training_text_length,
                           editor->current_token_index, editor->token_window_size, NUM_CHARS);
    }
}

