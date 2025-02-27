#pragma once

#include "Entity.h"

struct Plant : Entity
{
    Skeleton* skeleton;
};

Plant* 
CreatePlant(World* world, Vec2 pos);

void
CreateBranches(Skeleton* skeleton, int parent_index, Vec3 pos, R32 length, R32 radius, U32 color, int level, int max_level);

void
UpdatePlantSkeleton(Plant* plant);

void
RenderPlant(TiltedRenderer* renderer, Plant* plant);