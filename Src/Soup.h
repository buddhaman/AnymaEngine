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
    int state;
    VecR32 weights;

    // Pointers. This is inefficient, but its simple when we have multiple layers.
    Array<Neuron*> connections;
};

struct Soup
{
    // Input is simply part of the neuron array.
    Array<Neuron> neurons;
};