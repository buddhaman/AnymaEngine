#pragma once

#include "AnymUtil.h"

// Forward declarations
struct World;
struct Agent
{
    Vec2 pos; 
    Vec2 vel;

    R32 orientation;
    R32 radius;
};

void UpdateAgent(World* world, Agent* agent);