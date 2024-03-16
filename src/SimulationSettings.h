#pragma once

#include "AnymUtil.h"

struct SimulationSettings
{
    int max_agents = 8000;
    int n_initial_agents = 200;
    R32 chunk_size = 12.0f;
    int x_chunks = 60;
    int y_chunks = 60;
};
