#pragma once
#include "Linalg.h"
#include "Array.h"
#include <random>
#include <cmath>
#include <algorithm>

struct Neuron;

// Synapse with direct pointer to source neuron
struct Synapse
{
    Neuron* from;     // Direct pointer to source neuron (can be input or processing)
    R32 weight;
    R32 eligibility;  // eligibility trace
};

// Same neuron struct used for both input and processing neurons
struct Neuron
{
    int state;        // x_j(t) - current binary state: 0 or 1
    int prev_state;   // x_j(t-1) - previous state for causal Hebbian
    R32 bias;         // B_j - homeostatic bias (only used for processing neurons)
    R32 activation;   // u_j - current activation (only used for processing neurons)
    
    Array<Synapse> connections;  // Empty for input neurons
};

// Helper for sorting neurons by activation
struct NeuronActivation
{
    int index;
    R32 activation;
};

struct Soup
{
    Array<Neuron> inputs;   // Input layer - state set externally, no incoming connections
    Array<Neuron> neurons;  // Processing neurons with incoming connections
    
    // Global parameters
    R32 target_rate;        // ρ - fraction of processing neurons that fire each step (~0.1)
    R32 eta_bias;           // η_B - bias learning rate for homeostasis
    R32 eta_weight;         // η_w - weight learning rate
    R32 eligibility_decay;  // λ - eligibility decay
};

// Static random generator helpers
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

// L2 normalize incoming weights for a neuron
void NormalizeNeuronWeights(Neuron& neuron)
{
    R32 sum_sq = 0.0f;
    for(Synapse& syn : neuron.connections)
    {
        sum_sq += syn.weight * syn.weight;
    }
    if(sum_sq > 0.0f)
    {
        R32 inv_norm = 1.0f / sqrtf(sum_sq);
        for(Synapse& syn : neuron.connections)
        {
            syn.weight *= inv_norm;
        }
    }
}

// Update eligibility traces based on current states
// Call this AFTER setting correct output states for supervised learning
void UpdateEligibility(Soup& soup)
{
    // Causal Hebbian eligibility trace
    // e_ij(t) = λ * e_ij(t-1) + x_i(t-1) * x_j(t)
    for(Neuron& neuron : soup.neurons)
    {
        for(Synapse& syn : neuron.connections)
        {
            syn.eligibility = soup.eligibility_decay * syn.eligibility 
                            + syn.from->prev_state * neuron.state;
        }
    }
}

// Store current states as previous (call at end of step)
void StorePrevStates(Soup& soup)
{
    for(Neuron& input : soup.inputs)
    {
        input.prev_state = input.state;
    }
    for(Neuron& neuron : soup.neurons)
    {
        neuron.prev_state = neuron.state;
    }
}

void UpdateSoup(Soup& soup, bool update_eligibility = true)
{
    int num_neurons = (int)soup.neurons.size;
    if(num_neurons == 0) return;
    
    // Step 1: Compute activations for processing neurons
    // Input neurons keep their externally-set state
    for(Neuron& neuron : soup.neurons)
    {
        R32 u = 0.0f;
        for(Synapse& syn : neuron.connections)
        {
            u += syn.from->state * syn.weight;
        }
        neuron.activation = u + neuron.bias;
        neuron.state = 0;  // Reset states first
    }
    
    // Step 2: Select top k neurons by activation (enforced sparsity)
    int k = (int)(soup.target_rate * num_neurons);
    if(k < 1) k = 1;
    if(k > num_neurons) k = num_neurons;
    
    // Collect activations and find top-k
    static std::vector<NeuronActivation> activations;
    activations.resize(num_neurons);
    
    for(int i = 0; i < num_neurons; i++)
    {
        activations[i].index = i;
        activations[i].activation = soup.neurons[i].activation;
    }
    
    // Partial sort to find the k-th largest
    std::nth_element(activations.begin(), activations.begin() + k, activations.end(),
        [](const NeuronActivation& a, const NeuronActivation& b) {
            return a.activation > b.activation;  // Descending order
        });
    
    // Fire the top k neurons
    for(int i = 0; i < k; i++)
    {
        soup.neurons[activations[i].index].state = 1;
    }
    
    // Step 3: Eligibility update (can be skipped for supervised learning)
    if(update_eligibility)
    {
        UpdateEligibility(soup);
    }
    
    // Step 4: Bias homeostasis for processing neurons
    R32 rho = soup.target_rate;
    R32 eta_B = soup.eta_bias;
    R32 up_rate = eta_B * rho / (1.0f - rho);
    for(Neuron& neuron : soup.neurons)
    {
        if(neuron.state == 1)
        {
            neuron.bias -= eta_B;  // Fired: reduce boost
        }
        else
        {
            neuron.bias += up_rate;  // Didn't fire: increase boost
        }
    }
    
    // NOTE: StorePrevStates() must be called manually after UpdateSoup
    // This allows setting correct output states before eligibility update
}

// Apply dopamine-gated weight consolidation
// dopamine can be positive (reward) or negative (punishment)
// w_ij = w_ij + η_w * dopamine * e_ij, then L2 normalize
void ApplyDopamine(Soup& soup, R32 dopamine)
{
    if(dopamine == 0.0f) return;
    
    R32 eta_w = soup.eta_weight;
    for(Neuron& neuron : soup.neurons)
    {
        for(Synapse& syn : neuron.connections)
        {
            syn.weight += eta_w * dopamine * syn.eligibility;
        }
        NormalizeNeuronWeights(neuron);
    }
}

// Reset all eligibility traces to zero
void ResetEligibility(Soup& soup)
{
    for(Neuron& neuron : soup.neurons)
    {
        for(Synapse& syn : neuron.connections)
        {
            syn.eligibility = 0.0f;
        }
    }
}

// Set input neuron states
void SetInput(Soup& soup, int* input_states, int num_inputs)
{
    for(int i = 0; i < num_inputs && i < soup.inputs.size; i++)
    {
        soup.inputs[i].state = input_states[i];
    }
}

// Get output neuron states (last n processing neurons)
void GetOutput(Soup& soup, int* output_states, int num_outputs)
{
    I64 start = soup.neurons.size - num_outputs;
    if(start < 0) start = 0;
    for(int i = 0; i < num_outputs && (start + i) < soup.neurons.size; i++)
    {
        output_states[i] = soup.neurons[start + i].state;
    }
}

// Count how many processing neurons are currently firing
int CountActiveNeurons(Soup& soup)
{
    int count = 0;
    for(Neuron& neuron : soup.neurons)
    {
        count += neuron.state;
    }
    return count;
}

// Count how many input neurons are currently active
int CountActiveInputs(Soup& soup)
{
    int count = 0;
    for(Neuron& input : soup.inputs)
    {
        count += input.state;
    }
    return count;
}

Soup* CreateSoup(MemoryArena* arena, int num_input_neurons, int num_processing_neurons, int num_connections_per_neuron)
{
    Soup* soup = PushNewStruct(arena, Soup);
    soup->inputs = CreateArray<Neuron>(arena, num_input_neurons);
    soup->neurons = CreateArray<Neuron>(arena, num_processing_neurons);

    // Set default parameters
    soup->target_rate = 0.1f;       // 10% of processing neurons fire each step
    soup->eta_bias = 0.01f;         // Bias learning rate for homeostasis
    soup->eta_weight = 0.03f;
    soup->eligibility_decay = 0.95f;  // Slower decay for better credit assignment

    // Initialize input neurons (no connections)
    for(int i = 0; i < num_input_neurons; i++)
    {
        Neuron& input = *soup->inputs.PushBack();
        input.state = 0;
        input.prev_state = 0;
        input.bias = 0.0f;
        input.activation = 0.0f;
        input.connections = CreateArray<Synapse>(arena, 0);  // No incoming connections
    }

    // Initialize processing neurons (with connections)
    for(int i = 0; i < num_processing_neurons; i++)
    {
        Neuron& neuron = *soup->neurons.PushBack();
        neuron.state = 0;
        neuron.prev_state = 0;
        neuron.bias = 0.0f;
        neuron.activation = 0.0f;
        neuron.connections = CreateArray<Synapse>(arena, num_connections_per_neuron);
    }

    // Initialize connections for processing neurons
    // Each connection can point to either an input neuron or another processing neuron
    int total_sources = num_input_neurons + num_processing_neurons;
    for(int i = 0; i < num_processing_neurons; i++)
    {
        Neuron& neuron = soup->neurons[i];
        for(int j = 0; j < num_connections_per_neuron; j++)
        {
            Synapse& synapse = *neuron.connections.PushBack();
            int source_idx = RandomInt(0, total_sources - 1);
            if(source_idx < num_input_neurons)
            {
                synapse.from = &soup->inputs[source_idx];
            }
            else
            {
                synapse.from = &soup->neurons[source_idx - num_input_neurons];
            }
            synapse.weight = RandomR32(-1.0f, 1.0f);
            synapse.eligibility = 0.0f;
        }
        NormalizeNeuronWeights(neuron);
    }
    return soup;
}
