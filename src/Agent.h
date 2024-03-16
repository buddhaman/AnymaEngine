#pragma once

#include "AnymUtil.h"
#include "Array.h"
#include "Math.h"

// Forward declarations
struct World;

using EntityID = uint32_t;

enum AgentType
{
    AgentType_None = 0,
    AgentType_Herbivore = 1,
    AgentType_Carnivore = 2,
};

struct AgentEye
{
    Ray ray;
    R32 distance;
    AgentType hit_type;
    R32 orientation;
};

struct Agent
{
    Vec2 pos; 
    Vec2 vel;

    R32 orientation;
    R32 radius;

    Array<AgentEye> eyes;

    AgentType type;
    EntityID id;
    I32 ticks_until_reproduce;
    bool alive;
    U16 scratch_bits;  // Mark something on agents when you are sure you can do this. 
};

static inline U32
GetAgentColor(AgentType type)
{
    switch(type)
    {
    case AgentType_None: return 0xffffffff;
    case AgentType_Carnivore: return 0xff0000ff;
    case AgentType_Herbivore: return 0xff00ff00;
    }
    return 0xffffffff;
}

