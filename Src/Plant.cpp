#include "Plant.h"
#include "World.h"

static Plant* 
CreatePlant(World* world, Vec2 pos)
{
    MemoryArena* arena = world->plant_arena;
    Plant* plant = PushNewStruct(arena, Plant);
    world->plants.PushBack(plant);
    plant->skeleton = PushNewStruct(arena, Skeleton);
    plant->skeleton = CreateSkeleton(arena, 24, 64);
}

void
UpdatePlantSkeleton(Plant* plant)
{
    UpdateSkeleton(plant->skeleton);
}

void
RenderPlant(TiltedRenderer* renderer, Plant* plant)
{
    RenderSkeleton(renderer, plant->skeleton);
}
