#pragma once

#include "AnymUtil.h"

struct SimulationSettings
{
    int max_agents = 5000;
    int n_initial_agents = 1000;
    R32 chunk_size = 12.0f;
    int x_chunks = 60;
    int y_chunks = 60;
};
