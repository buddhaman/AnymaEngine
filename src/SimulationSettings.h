#pragma once

#include "AnymUtil.h"

struct SimulationSettings
{
    int max_agents = 20000;
    int n_initial_agents = 200;
    R32 chunk_size = 10.0f;
    int x_chunks = 120;
    int y_chunks = 120;
};
