#pragma once

#include "Entity.h"

struct Plant : Entity
{
    Skeleton* skeleton;
};

static Plant* 
CreatePlant(World* world, Vec2 pos);

void
UpdatePlantSkeleton(Plant* plant);

void
RenderPlant(TiltedRenderer* renderer, Plant* plant);