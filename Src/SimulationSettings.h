#pragma once

#include "AnymUtil.h"

#ifdef NDEBUG
constexpr int g_default_axis_chunks = 200;
constexpr int g_default_num_agents = 6000;
constexpr int g_default_max_agents = 6000;
#else
constexpr int g_default_axis_chunks = 50;
constexpr int g_default_num_agents = 100;
constexpr int g_default_max_agents = 100;
#endif

enum GamePhase
{
    GamePhase_SimulationScreen,
    GamePhase_EditorScreen,
};

constexpr I32 lifetime_multiplication_factor = 5;

struct SimulationSettings
{
    int max_agents = g_default_max_agents;
    int n_initial_agents = g_default_num_agents;
    R32 cell_size = 24.0f;
    int x_cells = g_default_axis_chunks;
    int y_cells = g_default_axis_chunks;

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