#pragma once

#include "AnymUtil.h"

struct Agent
{
    Vec2 pos; 
    Vec2 vel;

    R32 radius;
};

void Update(Agent* agent);