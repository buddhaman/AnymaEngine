#pragma once
#include "Linalg.h"
#include "Array.h"
#include <random>
#include <cmath>
#include <algorithm>

// CLEAN 3-LAYER ARCHITECTURE:
// Input (10 chars one-hot) -> Hidden (soup layer) -> Output (char prediction + next_token signal)
//
// Learning:
// - Hidden layer: Hebbian with 1-step delay (prev_state correlation), dopamine-gated
// - Output layer: Perceptron-style, learns more aggressively
// - Learning ONLY happens when predicting (when next_token fires)

struct Neuron;

struct Synapse
{
    Neuron* from;
    R32 weight;
    R32 eligibility;  // Causal Hebbian trace: pre(t-1) * post(t)
};

struct Neuron
{
    int state;       // 0 or 1
    int prev_state;  // Previous timestep
    R32 bias;
    R32 activation;
    Array<Synapse> connections;
};

struct Soup
{
    Array<Neuron> inputs;   // 310 neurons (10 tokens * 31 chars)
    Array<Neuron> hidden;   // ~1000 neurons
    Array<Neuron> outputs;  // 33 neurons: 31 char prediction + 2 next_token signal

    // Parameters
    R32 hidden_target_rate;    // ~0.1 (10% active)
    R32 eta_hidden_hebbian;    // Hebbian learning for hidden layer
    R32 eta_output_perceptron; // Perceptron learning for output layer
    R32 eta_bias;              // Bias homeostasis
    R32 eligibility_decay;     // Decay for Hebbian traces
};

// Random helpers
inline std::mt19937& StaticRng() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return gen;
}

inline int RandomInt(int a, int b) {
    std::uniform_int_distribution<int> dist(a, b);
    return dist(StaticRng());
}

inline R32 RandomR32(R32 a, R32 b) {
    std::uniform_real_distribution<R32> dist(a, b);
    return dist(StaticRng());
}

struct NeuronActivation
{
    int index;
    R32 activation;
};

// Create soup
Soup* CreateSoup(MemoryArena* arena, int num_input, int num_hidden, int num_output,
                int hidden_connections, int output_connections)
{
    Soup* soup = PushNewStruct(arena, Soup);

    soup->inputs = CreateArray<Neuron>(arena, num_input);
    soup->hidden = CreateArray<Neuron>(arena, num_hidden);
    soup->outputs = CreateArray<Neuron>(arena, num_output);

    // Parameters
    soup->hidden_target_rate = 0.1f;
    soup->eta_hidden_hebbian = 0.05f;
    soup->eta_output_perceptron = 0.3f;
    soup->eta_bias = 0.001f;
    soup->eligibility_decay = 0.95f;

    // Initialize input neurons
    for(int i = 0; i < num_input; i++)
    {
        Neuron& n = *soup->inputs.PushBack();
        n.state = 0;
        n.prev_state = 0;
        n.bias = 0.0f;
        n.activation = 0.0f;
        n.connections = CreateArray<Synapse>(arena, 0);
    }

    // Initialize hidden neurons (connect from inputs)
    for(int i = 0; i < num_hidden; i++)
    {
        Neuron& n = *soup->hidden.PushBack();
        n.state = 0;
        n.prev_state = 0;
        n.bias = 0.0f;
        n.activation = 0.0f;
        n.connections = CreateArray<Synapse>(arena, hidden_connections);

        for(int j = 0; j < hidden_connections; j++)
        {
            Synapse& syn = *n.connections.PushBack();
            syn.from = &soup->inputs[RandomInt(0, num_input - 1)];
            syn.weight = RandomR32(-0.5f, 0.5f);
            syn.eligibility = 0.0f;
        }
    }

    // Initialize output neurons (connect from hidden + inputs, heavily connected)
    int total_sources = num_input + num_hidden;
    for(int i = 0; i < num_output; i++)
    {
        Neuron& n = *soup->outputs.PushBack();
        n.state = 0;
        n.prev_state = 0;
        n.bias = 0.0f;
        n.activation = 0.0f;
        n.connections = CreateArray<Synapse>(arena, output_connections);

        for(int j = 0; j < output_connections; j++)
        {
            Synapse& syn = *n.connections.PushBack();
            int src_idx = RandomInt(0, total_sources - 1);
            if(src_idx < num_input)
                syn.from = &soup->inputs[src_idx];
            else
                syn.from = &soup->hidden[src_idx - num_input];

            syn.weight = RandomR32(-0.5f, 0.5f);
            syn.eligibility = 0.0f;
        }
    }

    return soup;
}

// Forward pass
void ForwardSoup(Soup& soup)
{
    // Hidden layer: winner-take-all
    for(Neuron& n : soup.hidden)
    {
        R32 u = 0.0f;
        for(Synapse& syn : n.connections)
            u += syn.from->state * syn.weight;
        n.activation = u + n.bias;
        n.state = 0;
    }

    int k = Max(1, (int)(soup.hidden_target_rate * soup.hidden.size));
    static std::vector<NeuronActivation> acts;
    acts.resize(soup.hidden.size);
    for(int i = 0; i < soup.hidden.size; i++)
    {
        acts[i].index = i;
        acts[i].activation = soup.hidden[i].activation;
    }
    std::nth_element(acts.begin(), acts.begin() + k, acts.end(),
        [](const NeuronActivation& a, const NeuronActivation& b) {
            return a.activation > b.activation;
        });
    for(int i = 0; i < k; i++)
        soup.hidden[acts[i].index].state = 1;

    // Output layer: compute activations
    for(Neuron& n : soup.outputs)
    {
        R32 u = 0.0f;
        for(Synapse& syn : n.connections)
            u += syn.from->state * syn.weight;
        n.activation = u + n.bias;
    }
}

// Update eligibility (causal Hebbian: pre(t-1) * post(t))
void UpdateEligibility(Soup& soup)
{
    for(Neuron& n : soup.hidden)
    {
        for(Synapse& syn : n.connections)
        {
            syn.eligibility = soup.eligibility_decay * syn.eligibility
                            + syn.from->prev_state * n.state;
        }
    }
}

// Normalize neuron weights (L1 normalization to keep activations in reasonable range)
void NormalizeNeuronWeights(Neuron& neuron)
{
    R32 sum_abs = 0.0f;
    for(Synapse& syn : neuron.connections)
        sum_abs += fabs(syn.weight);

    if(sum_abs > 0.0f)
    {
        R32 scale = (R32)neuron.connections.size / sum_abs;  // Scale to maintain average magnitude
        for(Synapse& syn : neuron.connections)
            syn.weight *= scale;
    }
}

// Apply Hebbian learning to hidden layer (dopamine-gated)
void ApplyHebbian(Soup& soup, R32 dopamine)
{
    R32 eta = soup.eta_hidden_hebbian;
    for(Neuron& n : soup.hidden)
    {
        for(Synapse& syn : n.connections)
        {
            syn.weight += eta * dopamine * syn.eligibility;
            syn.weight = Clamp(-3.0f, syn.weight, 3.0f);
        }
        // Removed NormalizeNeuronWeights - it was destroying learning!
    }
}

// Apply perceptron learning to output layer
void ApplyOutputLearning(Soup& soup, int* correct_pattern, bool was_correct)
{
    R32 eta = soup.eta_output_perceptron;

    for(int i = 0; i < soup.outputs.size; i++)
    {
        Neuron& out = soup.outputs[i];
        int target = correct_pattern[i];

        if(was_correct)
        {
            // Correct: reinforce active when should fire, suppress when shouldn't
            for(Synapse& syn : out.connections)
            {
                if(target == 1 && syn.from->state == 1)
                    syn.weight += eta;
                else if(target == 0 && syn.from->state == 1)
                    syn.weight -= eta * 0.5f;
            }
        }
        else
        {
            // Wrong: perceptron error correction
            R32 error = (R32)target - (R32)out.state;
            for(Synapse& syn : out.connections)
            {
                R32 input = (R32)syn.from->state;
                syn.weight += eta * error * input;
            }
            out.bias += eta * error;
        }

        for(Synapse& syn : out.connections)
            syn.weight = Clamp(-5.0f, syn.weight, 5.0f);
        out.bias = Clamp(-5.0f, out.bias, 5.0f);
        // Removed NormalizeNeuronWeights - it was destroying learning!
    }
}

// Homeostasis
void ApplyHomeostasis(Soup& soup)
{
    R32 rho = soup.hidden_target_rate;
    R32 eta = soup.eta_bias;
    R32 up_rate = eta * rho / (1.0f - rho);

    for(Neuron& n : soup.hidden)
    {
        if(n.state == 1)
            n.bias -= eta;
        else
            n.bias += up_rate;
    }
}

// Store prev states
void StorePrevStates(Soup& soup)
{
    for(Neuron& n : soup.inputs)
        n.prev_state = n.state;
    for(Neuron& n : soup.hidden)
        n.prev_state = n.state;
    for(Neuron& n : soup.outputs)
        n.prev_state = n.state;
}

// Count active neurons
int CountActiveHidden(Soup& soup)
{
    int count = 0;
    for(Neuron& n : soup.hidden)
        if(n.state == 1) count++;
    return count;
}

int CountActiveInputs(Soup& soup)
{
    int count = 0;
    for(Neuron& n : soup.inputs)
        if(n.state == 1) count++;
    return count;
}
