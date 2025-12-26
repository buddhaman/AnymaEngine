#pragma once
#include "Linalg.h"
#include "Array.h"
#include <random>
#include <cmath>

struct Neuron;

struct Synapse
{
    Neuron* from;
    R32 weight;
    R32 eligibility;  // eligibility trace
};

// Neurons are sparsely connected. Number of weights is arbitrary.
struct Neuron
{
    int state;        // x_j(t) - current binary state: 0 or 1
    int prev_state;   // x_j(t-1) - previous state for causal Hebbian
    R32 threshold;    // B_j, homeostatic threshold
    
    // Pointers. This is inefficient, but its simple when we have multiple layers.
    Array<Synapse> connections;
};

struct Soup
{
    Array<Neuron> neurons;
    
    // Global parameters
    R32 target_rate;        // ρ - target firing rate (~0.1)
    R32 eta_threshold;      // η_B - threshold learning rate
    R32 eta_weight;         // η_w - weight learning rate (η_w << η_B)
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

void UpdateSoup(Soup& soup)
{
    // Step 1: Compute neuron inputs and fire
    // u_j(t) = Σ w_ij * x_i(t), x_j(t) = 1[u_j(t) > B_j]
    for(Neuron& neuron : soup.neurons)
    {
        R32 u = 0.0f;
        for(Synapse& syn : neuron.connections)
        {
            u += syn.from->state * syn.weight;
        }
        neuron.state = (u > neuron.threshold) ? 1 : 0;
    }
    
    // Step 2: Causal Hebbian eligibility trace
    // e_ij(t) = λ * e_ij(t-1) + x_i(t-1) * x_j(t)
    for(Neuron& neuron : soup.neurons)
    {
        for(Synapse& syn : neuron.connections)
        {
            syn.eligibility = soup.eligibility_decay * syn.eligibility 
                            + syn.from->prev_state * neuron.state;
        }
    }
    
    // Step 3: Threshold homeostasis
    // B_j += η_B if x_j=1, B_j -= η_B * ρ/(1-ρ) if x_j=0
    R32 rho = soup.target_rate;
    R32 eta_B = soup.eta_threshold;
    R32 down_rate = eta_B * rho / (1.0f - rho);
    for(Neuron& neuron : soup.neurons)
    {
        if(neuron.state == 1)
        {
            neuron.threshold += eta_B;
        }
        else
        {
            neuron.threshold -= down_rate;
        }
    }
    
    // Step 4: Store current state as previous for next timestep
    for(Neuron& neuron : soup.neurons)
    {
        neuron.prev_state = neuron.state;
    }
}

// Apply dopamine-gated weight consolidation
// dopamine ∈ [0,1] - continuous reward signal (e.g. model's probability for true token)
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

// Set input neurons (first n neurons)
void SetInput(Soup& soup, int* input_states, int num_inputs)
{
    for(int i = 0; i < num_inputs && i < soup.neurons.size; i++)
    {
        soup.neurons[i].state = input_states[i];
    }
}

// Get output neuron states (last n neurons)
void GetOutput(Soup& soup, int* output_states, int num_outputs)
{
    I64 start = soup.neurons.size - num_outputs;
    if(start < 0) start = 0;
    for(int i = 0; i < num_outputs && (start + i) < soup.neurons.size; i++)
    {
        output_states[i] = soup.neurons[start + i].state;
    }
}

// Count how many neurons are currently firing
int CountActiveNeurons(Soup& soup)
{
    int count = 0;
    for(Neuron& neuron : soup.neurons)
    {
        count += neuron.state;
    }
    return count;
}

Soup* CreateSoup(MemoryArena* arena, int num_neurons, int num_connections_per_neuron)
{
    Soup* soup = PushNewStruct(arena, Soup);
    soup->neurons = CreateArray<Neuron>(arena, num_neurons);

    // Set default parameters
    soup->target_rate = 0.1f;
    soup->eta_threshold = 0.01f;
    soup->eta_weight = 0.001f;
    soup->eligibility_decay = 0.9f;

    // First initialize all neurons (without connections)
    for(int i = 0; i < num_neurons; i++)
    {
        Neuron& neuron = *soup->neurons.PushBack();
        neuron.state = 0;
        neuron.prev_state = 0;
        neuron.threshold = 0.1f;
        neuron.connections = CreateArray<Synapse>(arena, num_connections_per_neuron);
    }

    // Now that all neurons are allocated, initialize connections
    for (int i = 0; i < num_neurons; i++)
    {
        Neuron& neuron = soup->neurons[i];
        for (int j = 0; j < num_connections_per_neuron; j++)
        {
            Synapse& synapse = *neuron.connections.PushBack();
            synapse.from = &soup->neurons[RandomInt(0, num_neurons-1)];
            synapse.weight = RandomR32(-1.0f, 1.0f);
            synapse.eligibility = 0.0f;
        }
        NormalizeNeuronWeights(neuron);
    }
    return soup;
}