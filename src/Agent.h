#pragma once

#include "AnymUtil.h"
#include "Array.h"
#include "Math.h"
#include "Linalg.h"

// Forward declarations
struct World;

using EntityID = U32;

enum AgentType
{
    AgentType_None = 0,
    AgentType_Herbivore = 1,
    AgentType_Carnivore = 2,
    AgentType_Num
};

struct AgentEye
{
    Ray ray;
    R32 distance;
    AgentType hit_type;
    R32 orientation;
};

struct Brain
{
    VecR32 input;
    MatR32 weights;
    VecR32 output;
    VecR32 gene;
};

struct Agent
{
    Vec2 pos; 
    Vec2 vel;

    R32 orientation;
    R32 radius;
    I32 charge;
    I32 charge_refractory;

    Array<AgentEye> eyes;
    Brain* brain;

    AgentType type;
    EntityID id;

    I32 ticks_until_reproduce;
    I32 energy;
    I32 ticks_alive;

    // This is the color used to identify the gene. For visualization.
    U32 gene_color;

    bool is_alive;
};

static inline U32
GetAgentColor(AgentType type)
{
    switch(type)
    {
    case AgentType_Carnivore: return RGBAColor(255, 0, 0, 255);
    case AgentType_Herbivore: return RGBAColor(0, 255, 0, 255);
    default: return 0xffffffff;
    }
}

static inline void
UpdateBrain(Agent* agent)
{
    Brain* brain = agent->brain;
    MatR32VecMul(brain->output, brain->weights, brain->input);
    brain->output.Apply([](R32 x) -> R32 {return tanh(x);});
}

void
UpdateMovement(World* world, Agent* agent);

I32
GetTicksUntilReproduction(World* world, AgentType type);

I32
GetInitialEnergy(World* world, AgentType type);

static inline bool
IsCharging(Agent* agent)
{
    return agent->charge;
}

static inline bool
IsRefractory(Agent* agent)
{
    return agent->charge_refractory != 0;
}



