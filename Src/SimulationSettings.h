#pragma once

#include "AnymUtil.h"

#ifdef NDEBUG
#define DEFAULT_AXIS_CHUNKS 200
#define DEFAULT_NUM_AGENTS 6000
#define DEFAULT_MAX_AGENTS 6000
#else
#define DEFAULT_AXIS_CHUNKS 10
#define DEFAULT_NUM_AGENTS 100
#define DEFAULT_MAX_AGENTS 100
#endif

enum GamePhase
{
    GamePhase_SimulationScreen,
    GamePhase_EditorScreen,
};

struct SimulationSettings
{
    int max_agents = DEFAULT_MAX_AGENTS;
    int n_initial_agents = DEFAULT_NUM_AGENTS;
    R32 chunk_size = 12.0f;
    int x_chunks = DEFAULT_AXIS_CHUNKS;
    int y_chunks = DEFAULT_AXIS_CHUNKS;

    R32 mutation_rate = 0.012f;
    I32 energy_transfer_on_hit = 350;
    I32 charge_duration = 20;     // 60 per second.
    I32 charge_refractory_period = 240;     // 60 per second.

    I32 herbivore_reproduction_ticks = 60*10;
    I32 herbivore_initial_energy = 60*11;;

    I32 carnivore_reproduction_ticks = 60*26;
    I32 carnivore_initial_energy = 60*25;

    // Current phase settings
    GamePhase current_phase = GamePhase_EditorScreen;
};

inline SimulationSettings global_settings;