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

Verlet3* 
AddParticle(Skeleton* skeleton, Vec3 pos);

void
UpdateSkeleton(Skeleton* skeleton);

Verlet3Constraint* 
Connect(Skeleton* skeleton, int idx0, int idx1);

void
RenderSkeleton(TiltedRenderer* renderer, Skeleton* skeleton);

Skeleton*
CreateSkeleton(MemoryArena* arena, Vec3 pos, R32 scale);