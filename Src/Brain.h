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

static inline I64 
CalculateBrainSizeFeedForward(int inputSize, int hiddenSize, int outputSize)
{
    return (inputSize * hiddenSize + hiddenSize) + (hiddenSize * outputSize + outputSize);
}

Brain* 
CreateBrain(MemoryArena* arena, int inputSize, int hiddenSize, int outputSize, Brain* parent, R32 mutationRate)
{
    Brain* brain = PushNewStruct(arena, Brain);

    I64 geneSize = CalculateBrainSizeFeedForward(inputSize, hiddenSize, outputSize);

    brain->gene = VecR32Create(arena, geneSize);

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
    brain->gene.AddNormal(0, mutationRate);

    // Create input, hidden, and output vectors
    brain->input = VecR32Create(arena, inputSize);
    brain->hidden = VecR32Create(arena, hiddenSize);
    brain->output = VecR32Create(arena, outputSize);

    // Shape gene into matrices and biases for input-hidden and hidden-output layers
    I64 offset = 0;
    brain->input_weights = brain->gene.ShapeAs(hiddenSize, inputSize, offset);
    brain->input_biases = VecR32Create(hiddenSize, brain->gene.v + offset);
    offset += hiddenSize;

    brain->hidden_weights = brain->gene.ShapeAs(outputSize, hiddenSize, offset);
    brain->hidden_biases = VecR32Create(outputSize, brain->gene.v + offset);

    return brain;
}

static inline void UpdateBrain(Brain* brain)
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
