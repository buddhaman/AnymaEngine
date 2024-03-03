#pragma once

#include "AnymUtil.h"

struct Agent
{
    Vec2 pos; 
    R32 radius;
    Vec2 vel;
};

void Update(Agent* agent);