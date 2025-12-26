#pragma once
#include "Linalg.h"
#include "Array.h"

struct Neuron;

struct Synapse
{
    Neuron* from;
    R32 weight;
    R32 weight_before_dopamine;
};

// Neurons are sparsely connected. Number of weights is arbitrary.
struct Neuron
{
    // only 0 or 1
    int state;
    R32 bias = 0.1;

    // Pointers. This is inefficient, but its simple when we have multiple layers.
    Array<Synapse> connections;
};

struct Soup
{
    // Input is simply part of the neuron array.
    Array<Neuron> neurons;
    R32 target_activation = 0.1f;
};

void UpdateSoup(Soup& soup)
{
    for(Neuron& neuron : soup.neurons)
    {
        R32 sum = 0; 
        for(Synapse& synapse : neuron.connections)
        {
            sum += synapse.from->state * synapse.weight;
        }
        neuron.state = sum;
    }
}

Soup* CreateSoup(MemoryArena* arena, int num_neurons, int num_connections_per_neuron)
{
    Soup* soup = PushNewStruct(arena, Soup);
    soup->neurons = CreateArray<Neuron>(arena, num_neurons);
    for(int i = 0; i < num_neurons; i++)
    {
        Neuron& neuron = *soup->neurons.PushBack();
        neuron.state = 0;
        neuron.bias = 0.1f;
        neuron.connections = CreateArray<Synapse>(arena, num_connections_per_neuron);
        for(int j = 0; j < num_connections_per_neuron; j++)
        {
            Synapse& synapse = *neuron.connections.PushBack();
            synapse.from = &soup->neurons[RandomInt(0, num_neurons-1)];
            synapse.weight = RandomR32(0, 1);
            synapse.weight_before_dopamine = synapse.weight;
        }
    }
    return soup;
}