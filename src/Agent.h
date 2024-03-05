#pragma once

#include "AnymUtil.h"

// Forward declarations
struct World;

enum AgentType
{
    AgentType_Herbivore = 0,
    AgentType_Carnivore,
};

struct Agent
{
    Vec2 pos; 
    Vec2 vel;

    R32 orientation;
    R32 radius;

    AgentType type;
};
