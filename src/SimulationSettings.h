#pragma once

#include "AnymUtil.h"

#ifdef NDEBUG
#define DEFAULT_AXIS_CHUNKS 200
#else
#define DEFAULT_AXIS_CHUNKS 60
#endif

struct SimulationSettings
{
    int max_agents = 5000;
    int n_initial_agents = 1000;
    R32 chunk_size = 12.0f;
    int x_chunks = DEFAULT_AXIS_CHUNKS;
    int y_chunks = DEFAULT_AXIS_CHUNKS;
    R32 default_mutation_rate = 0.03f;
};
