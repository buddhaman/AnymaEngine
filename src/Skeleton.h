#pragma once

#include "Verlet3.h"
#include "Array.h"

// This is the detailed struct that you see in high LOD.
struct Skeleton
{
    Array<Verlet3> particles;
    Array<Verlet3Constraint> constraints;
};

Skeleton*
CreateSkeleton(MemoryArena* arena, Vec3 pos, R32 scale);