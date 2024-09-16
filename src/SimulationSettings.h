#pragma once

#include "AnymUtil.h"

#ifdef NDEBUG
#define DEFAULT_AXIS_CHUNKS 200
#define DEFAULT_NUM_AGENTS 6000
#define DEFAULT_MAX_AGENTS 6000
#else
#define DEFAULT_AXIS_CHUNKS 120
#define DEFAULT_NUM_AGENTS 5000
#define DEFAULT_MAX_AGENTS 5000
#endif

struct SimulationSettings
{
    int max_agents = DEFAULT_MAX_AGENTS;
    int n_initial_agents = DEFAULT_NUM_AGENTS;
    R32 chunk_size = 12.0f;
    int x_chunks = DEFAULT_AXIS_CHUNKS;
    int y_chunks = DEFAULT_AXIS_CHUNKS;
    R32 default_mutation_rate = 0.004f;

    I32 energy_transfer_on_hit = 4;
    I32 charge_duration = 60;     // 60 per seconds.
    I32 charge_refractory_period = 120;     // 60 per seconds.
};

inline SimulationSettings global_settings;