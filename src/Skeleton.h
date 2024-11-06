#pragma once

#include "Verlet3.h"
#include "Array.h"
#include "Renderer.h"

struct Joint
{
    Verlet3* v;
    R32 r;
    U32 color;
};

struct Limb
{
    Verlet3Constraint* constraint;
    R32 from_width;
    R32 to_width;
    U32 color;
};

struct Leg
{
    // Idx of the end of the leg.
    int idx;    
    Vec3 offset;
};

struct Head
{
    // should always be 0. But just in case.
    int idx;

    Vec3 target_position;
};

struct MainBody
{
    Array<int> body;
    Array<Vec3> target_positions;
};

// TODO: Add tail, mouth etc

// This is the detailed struct that you see in high LOD.
struct Skeleton
{
    Array<Verlet3> particles;
    Array<Verlet3Constraint> constraints;

    Array<Joint> joints;
    Array<Limb> limbs;

    Head head;
    MainBody body;
    Array<Leg> legs;
};

Verlet3* 
AddParticle(Skeleton* skeleton, Vec3 pos);

Joint* 
AddJoint(Skeleton* skeleton, Vec3 pos, R32 r, U32 color);

void
UpdateSkeleton(Skeleton* skeleton);

Verlet3Constraint* 
Connect(Skeleton* skeleton, int idx0, int idx1);

Limb* 
Connect(Skeleton* skeleton, int idx0, R32 width0, int idx1, R32 width1, U32 color);

void
RenderSkeleton(TiltedRenderer* renderer, Skeleton* skeleton);

Skeleton*
CreateSkeleton(MemoryArena* arena, int max_joints, int max_limbs);