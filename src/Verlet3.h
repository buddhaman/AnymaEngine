#pragma once

#include "TimVectorMath.h"
#include "AnymUtil.h"

// TODO(SIMD): Optimize in big batches!!!.

struct Verlet3
{
    Vec3 pos;
    Vec3 old_pos;
};

struct Verlet3Constraint
{
    Verlet3* v0;
    Verlet3* v1;
    R32 r;
};

// Friction is technically 1.0-friction. But this is my game I make the definitions HAHAHAHAHAH.
static inline void 
VerletUpdate(Verlet3* particle, float friction = 1.0f)
{
    Vec3 velocity = particle->pos - particle->old_pos;
    velocity *= friction;
    particle->old_pos = particle->pos;
    particle->pos += velocity;
}

static inline void
VerletSolve(Verlet3Constraint* constraint)
{
    Vec3 delta = constraint->v1->pos - constraint->v0->pos;
    float distance = V3Len(delta);

    if (distance == 0.0f) return;

    float correction = (distance - constraint->r) / distance;
    Vec3 correction_vector = delta * 0.5f * correction;
    constraint->v0->pos += correction_vector; 
    constraint->v1->pos -= correction_vector;
}

static inline void
AddImpulse(Verlet3* v, Vec3 impulse)
{
    v->old_pos -= impulse;
}