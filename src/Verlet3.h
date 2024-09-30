#pragma once

#include "TimVectorMath.h"
#include "AnymUtil.h"

// TODO(SIMD): Optimize in big batches!!!.

struct Verlet3
{
    Vec3 pos;
    Vec3 old_pos;
};

struct VerletConstraint
{
    Verlet3* v0;
    Verlet3* v1;
    R32 r;
};

// Friction is technically 1.0-friction. But this is my game I make the definitions HAHAHAHAHAH.
void VerletUpdate(Verlet3* particle, float friction = 1.0f)
{
    // Calculate velocity as the difference between the current and old position
    Vec3 velocity = particle->pos - particle->old_pos;
    velocity *= friction;
    particle->old_pos = particle->pos;
    particle->pos += velocity;
}

void 
VerletSolve(VerletConstraint* constraint)
{
    // Calculate the vector between the two particles
    Vec3 delta = constraint->v1->pos - constraint->v0->pos;
    float distance = V3Len(delta);

    if (distance == 0.0f) return;

    float correction = (distance - constraint->r) / distance;
    Vec3 correction_vector = delta * 0.5f * correction;
    constraint->v0->pos += correction_vector; 
    constraint->v1->pos -= correction_vector;
}
