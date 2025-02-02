#pragma once

#include "AnymUtil.h"

#ifdef NDEBUG
#define DEFAULT_AXIS_CHUNKS 200
#define DEFAULT_NUM_AGENTS 6000
#define DEFAULT_MAX_AGENTS 6000
#else
#define DEFAULT_AXIS_CHUNKS 50
#define DEFAULT_NUM_AGENTS 100
#define DEFAULT_MAX_AGENTS 100
#endif

enum GamePhase
{
    GamePhase_SimulationScreen,
    GamePhase_EditorScreen,
};

constexpr I32 lifetime_multiplication_factor = 5;

struct SimulationSettings
{
    int max_agents = DEFAULT_MAX_AGENTS;
    int n_initial_agents = DEFAULT_NUM_AGENTS;
    R32 cell_size = 24.0f;
    int x_cells = DEFAULT_AXIS_CHUNKS;
    int y_cells = DEFAULT_AXIS_CHUNKS;

    R32 mutation_rate = 0.012f;
    I32 energy_transfer_on_hit = 350;
    I32 charge_duration = 20;     // 60 per second.
    I32 charge_refractory_period = 240;     // 60 per second.

    I32 herbivore_reproduction_ticks = 60*10*lifetime_multiplication_factor;
    I32 herbivore_initial_energy = 60*11*lifetime_multiplication_factor;

    I32 carnivore_reproduction_ticks = 60*26*lifetime_multiplication_factor;
    I32 carnivore_initial_energy = 60*25*lifetime_multiplication_factor;

    I32 max_lifespan = 60*60*lifetime_multiplication_factor;

    // Current phase settings
    GamePhase current_phase = GamePhase_EditorScreen;
};

inline SimulationSettings global_settings;