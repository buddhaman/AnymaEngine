#pragma once

#include "Verlet3.h"
#include "Array.h"
#include "Renderer.h"

// This is the detailed struct that you see in high LOD.
struct Skeleton
{
    Array<Verlet3> particles;
    Array<Verlet3Constraint> constraints;
};

void
AddParticle(Skeleton* skeleton, Vec3 pos);

void
Update(Skeleton* skeleton);

void
Render(Renderer* renderer, Skeleton* skeleton);

Skeleton*
CreateSkeleton(MemoryArena* arena, Vec3 pos, R32 scale);