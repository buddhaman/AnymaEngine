#pragma once

#include "AnymUtil.h"
#include "Array.h"

// Genome design:
// Collection of nodes with references like karl sims? 
// Node (Head?): 
//  - Color differential.
//  - Size differential.
//  - Angle differential.
//  - Distance differential (length of bodypart).
//  - Children.
// Each child is offset with the differentials. Some of these are obviously ignored for the root node.
// Do we have types for mouth / eyes etc?
struct GeneNode
{
    R32 hue_diff;
    R32 sat_diff;
    R32 brightness_diff;

    R32 ang_diff;
    R32 len_diff;

    // Store children as indices.
    Array<int> children;
};

Array<GeneNode>
CreateGenes(MemoryArena* arena, int n_genes);

