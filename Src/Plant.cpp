#include "Plant.h"
#include "World.h"

Plant* 
CreatePlant(World* world, Vec2 pos)
{
    MemoryArena* arena = world->plant_arena;
    Plant* plant = PushNewStruct(arena, Plant);
    world->plants.PushBack(plant);
    plant->skeleton = PushNewStruct(arena, Skeleton);
    plant->skeleton = CreateSkeleton(arena, 24, 64);

    R32 r = 0.5f;
    R32 dist = 4.0f;
    U32 color = Vec4ToColor(V4(
        RandomR32Debug(0.0f, 1.0f),
        RandomR32Debug(0.0f, 1.0f),
        RandomR32Debug(0.0f, 1.0f),
        1.0f
    ));
    AddJoint(plant->skeleton, V3(pos.x, pos.y, 0.0f), r, color);
    AddJoint(plant->skeleton, V3(pos.x, pos.y, dist*1.0f), r*0.8f, color);
    AddJoint(plant->skeleton, V3(pos.x, pos.y, dist*2.0f), r*0.8f, color);
    Connect(plant->skeleton, 0, r, 1, r*0.8f, color);
    Connect(plant->skeleton, 1, r*0.8f, 2, r*0.8f, color);

    return plant;
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
