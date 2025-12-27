#pragma once
#include "Linalg.h"
#include "Array.h"
#include <random>
#include <cmath>
#include <algorithm>

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
    R32 bias;         // B_j - homeostatic bias (added to activation)
    R32 activation;   // u_j - current activation = input + bias
    
    // Pointers. This is inefficient, but its simple when we have multiple layers.
    Array<Synapse> connections;
};

// Helper for sorting neurons by activation
struct NeuronActivation
{
    int index;
    R32 activation;
};

struct Soup
{
    Array<Neuron> neurons;
    
    // Global parameters
    R32 target_rate;        // ρ - fraction of neurons that fire each step (~0.1)
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

void UpdateSoup(Soup& soup)
{
    int num_neurons = (int)soup.neurons.size;
    if(num_neurons == 0) return;
    
    // Step 1: Compute activations for all neurons
    // activation = input_sum + bias (bias acts as homeostatic boost)
    for(Neuron& neuron : soup.neurons)
    {
        R32 u = 0.0f;
        for(Synapse& syn : neuron.connections)
        {
            u += syn.from->state * syn.weight;
        }
        neuron.activation = u + neuron.bias;
        neuron.state = 0;  // Reset all states first
    }
    
    // Step 2: Select top k neurons by activation (enforced sparsity)
    // k = target_rate * num_neurons (at most p fraction can fire)
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
    
    // Step 3: Causal Hebbian eligibility trace
    // e_ij(t) = λ * e_ij(t-1) + x_i(t-1) * x_j(t)
    for(Neuron& neuron : soup.neurons)
    {
        for(Synapse& syn : neuron.connections)
        {
            syn.eligibility = soup.eligibility_decay * syn.eligibility 
                            + syn.from->prev_state * neuron.state;
        }
    }
    
    // Step 4: Bias homeostasis
    // If fired: decrease bias (less boost next time, harder to compete)
    // If didn't fire: increase bias (more boost next time, easier to compete)
    // This self-balances: inactive neurons get boosted, overactive ones get dampened
    R32 rho = soup.target_rate;
    R32 eta_B = soup.eta_bias;
    R32 up_rate = eta_B * rho / (1.0f - rho);  // Rate for neurons that didn't fire
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
    
    // Step 5: Store current state as previous for next timestep
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
    soup->target_rate = 0.1f;       // 10% of neurons fire each step
    soup->eta_bias = 0.01f;         // Bias learning rate for homeostasis
    soup->eta_weight = 0.03f;
    soup->eligibility_decay = 0.9f;

    // First initialize all neurons (without connections)
    for(int i = 0; i < num_neurons; i++)
    {
        Neuron& neuron = *soup->neurons.PushBack();
        neuron.state = 0;
        neuron.prev_state = 0;
        neuron.bias = 0.0f;         // Start with no bias
        neuron.activation = 0.0f;
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