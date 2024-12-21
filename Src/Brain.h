#pragma once

#include "Linalg.h"

struct Brain
{
    // Weights and biases.
    MatR32 input_weights;
    VecR32 input_biases;
    MatR32 hidden_weights;
    VecR32 hidden_biases;

    // Mutable
    VecR32 input;
    VecR32 hidden;
    VecR32 output;

    VecR32 gene;
};

static inline int
CalculateBrainSizeFeedForward(int inputSize, int hiddenSize, int outputSize)
{
    return (inputSize * hiddenSize + hiddenSize) + (hiddenSize * outputSize + outputSize);
}

Brain* 
CreateBrain(MemoryArena* arena, 
            int input_size, 
            int hidden_size, 
            int output_size, 
            Brain* parent, 
            R32 mutation_rate)
{
    Brain* brain = PushNewStruct(arena, Brain);

    int gene_size = CalculateBrainSizeFeedForward(input_size, hidden_size, output_size);

    brain->gene = VecR32Create(arena, gene_size);

    if (parent)
    {
        brain->gene.CopyFrom(parent->gene);
    }
    else
    {
        brain->gene.Set(0);
        brain->gene.AddNormal(0, 0.5f);
    }

    // Apply mutations to the gene
    brain->gene.AddNormal(0, mutation_rate);

    // Create input, hidden, and output vectors
    brain->input = VecR32Create(arena, input_size);
    brain->hidden = VecR32Create(arena, hidden_size);
    brain->output = VecR32Create(arena, output_size);

    // Shape gene into matrices and biases for input-hidden and hidden-output layers
    I64 offset = 0;
    brain->input_weights = brain->gene.ShapeAs(input_size, hidden_size, offset);
    brain->input_biases = VecR32Create(hidden_size, brain->gene.v + offset);
    offset += hidden_size;

    brain->hidden_weights = brain->gene.ShapeAs(hidden_size, output_size, offset);
    brain->hidden_biases = VecR32Create(output_size, brain->gene.v + offset);

    offset+=output_size;

    Assert(offset == brain->gene.n && "Gene size mismatch during creation.");

    return brain;
}

static inline void 
UpdateBrain(Brain* brain)
{
    MatR32VecMul(brain->hidden, brain->input_weights, brain->input);

    for (int i = 0; i < brain->hidden.n; i++)
    {
        brain->hidden.v[i] += brain->input_biases.v[i];
    }

    brain->hidden.Apply([](R32 x) -> R32 { return tanh(x); });

    MatR32VecMul(brain->output, brain->hidden_weights, brain->hidden);

    for (int i = 0; i < brain->output.n; i++)
    {
        brain->output.v[i] += brain->hidden_biases.v[i];
    }

    brain->output.Apply([](R32 x) -> R32 { return tanh(x); });
}
