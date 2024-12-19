#pragma once

#include "Linalg.h"

struct Brain
{
    MatR32 input_weights;
    MatR32 hidden_weights;
    VecR32 input;
    VecR32 hidden;
    VecR32 output;
    VecR32 gene;
};

static inline I64 
CalculateBrainGeneSizeFeedForward(int inputSize, int hiddenSize, int outputSize)
{
    // Gene size is the sum of weights between input and hidden, and hidden and output
    return inputSize * hiddenSize + hiddenSize * outputSize;
}

static inline void
UpdateBrain(Brain* brain)
{
    MatR32VecMul(brain->output, brain->weights, brain->input);
    brain->output.Apply([](R32 x) -> R32 {return tanh(x);});
}

// Parent = nullptr if no parent.
Brain* 
CreateBrainFeedForward(MemoryArena* arena, int inputSize, int hiddenSize, int outputSize, VecR32* parent, R32 mutationRate)
{
    // Allocate memory for the Brain struct
    Brain* brain = PushNewStruct(arena, Brain);

    // Precalculate gene size
    I64 gene_size = CalculateBrainGeneSizeFeedForward(inputSize, hiddenSize, outputSize);

    // Create the gene vector
    brain->gene = VecR32Create(arena, gene_size);

    // If a parent exists, copy its gene, otherwise initialize with random values
    if (parent)
    {
        Assert(parent->n==gene_size);
        brain->gene.CopyFrom(parent);
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
    brain->hidden = VecR32Create(arena, hiddenSize); // Hidden layer added
    brain->output = VecR32Create(arena, outputSize);

    // Shape gene into matrices for input-hidden and hidden-output weights
    I64 offset = 0;
    brain->input_weights = brain->gene.ShapeAs(hiddenSize, inputSize, offset);
    brain->hidden_weights = brain->gene.ShapeAs(outputSize, hiddenSize, offset);

    return brain;
}
