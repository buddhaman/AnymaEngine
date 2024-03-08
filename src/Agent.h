#pragma once

#include "AnymUtil.h"
#include "Array.h"
#include "Math.h"

// Forward declarations
struct World;

using EntityID = uint32_t;

enum AgentType
{
    AgentType_Herbivore = 0,
    AgentType_Carnivore,
};

struct AgentEye
{
    Ray ray;
    R32 distance;
    U32 color;
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
};
