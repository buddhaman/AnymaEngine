#pragma once

#include "AnymUtil.h"

#ifdef NDEBUG
#define DEFAULT_AXIS_CHUNKS 200
#define DEFAULT_NUM_AGENTS 6000
#define DEFAULT_MAX_AGENTS 6000
#else
#define DEFAULT_AXIS_CHUNKS 50
#define DEFAULT_NUM_AGENTS 1000
#define DEFAULT_MAX_AGENTS 1000
#endif

struct SimulationSettings
{
    int max_agents = DEFAULT_MAX_AGENTS;
    int n_initial_agents = DEFAULT_NUM_AGENTS;
    R32 chunk_size = 12.0f;
    int x_chunks = DEFAULT_AXIS_CHUNKS;
    int y_chunks = DEFAULT_AXIS_CHUNKS;
    R32 mutation_rate = 0.004f;

    I32 energy_transfer_on_hit = 350;
    I32 charge_duration = 20;     // 60 per second.
    I32 charge_refractory_period = 240;     // 60 per second.
};

inline SimulationSettings global_settings;