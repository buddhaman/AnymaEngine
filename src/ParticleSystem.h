#pragma once

#include "TimVectorMath.h"
#include "AnymUtil.h"

struct Particle
{
    Vec2 pos;
    Vec2 vel;
};

struct ParticleSystem
{
    Particle particles[256];
};

