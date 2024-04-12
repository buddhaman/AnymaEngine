#pragma once

#include "AnymUtil.h"

#ifdef NDEBUG
#define DEFAULT_AXIS_CHUNKS 200
#define DEFAULT_NUM_AGENTS 6000
#define DEFAULT_MAX_AGENTS 6000
#else
#define DEFAULT_AXIS_CHUNKS 120
#define DEFAULT_NUM_AGENTS 6000
#define DEFAULT_MAX_AGENTS 1000
#endif

struct SimulationSettings
{
    int max_agents = 5000;
    int n_initial_agents = 1000;
    R32 chunk_size = 12.0f;
    int x_chunks = DEFAULT_AXIS_CHUNKS;
    int y_chunks = DEFAULT_AXIS_CHUNKS;
    R32 default_mutation_rate = 0.004f;
};
