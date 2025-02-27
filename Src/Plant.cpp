#include "Plant.h"
#include "World.h"

Plant*
CreatePlant(World* world, Vec2 pos)
{
    MemoryArena* arena = world->plant_arena;
    Plant* plant = PushNewStruct(arena, Plant);
    world->plants.PushBack(plant);
    
    plant->skeleton = CreateSkeleton(arena, 50, 64);

    R32 base_radius = 0.5f;
    R32 base_length = 4.0f;
    U32 color = Vec4ToColor(V4(
        RandomR32Debug(0.0f, 1.0f),
        RandomR32Debug(0.0f, 1.0f),
        RandomR32Debug(0.0f, 1.0f),
        1.0f
    ));

    // Root joint at the ground position
    AddJoint(plant->skeleton, V3(pos.x, pos.y, 0.0f), base_radius, color);
    
    // Generate branching structure
    CreateBranches(plant->skeleton, 0, V3(pos.x, pos.y, 0.0f), base_length, base_radius, color, 0, 3);

    return plant;
}

void
CreateBranches(Skeleton* skeleton, int parent_index, Vec3 pos, R32 length, R32 radius, U32 color, int level, int max_level)
{
    if (level >= max_level) return;

    int num_branches = (level == 0) ? 3 : 3;
    R32 next_radius = radius * 0.7f; 
    R32 next_length = length * 0.8f;

    for (int i = 0; i < num_branches; ++i)
    {
        R32 angle = DegToRad(i * (360.0f / num_branches)); 

        Vec3 direction = V3(
            length * cosf(angle),  // Spread in X
            length * sinf(angle),  // Spread in Y
            length                 // Always grow upwards in Z
        );

        Vec3 child_pos = pos + direction;

        int child_index = skeleton->joints.size; 
        AddJoint(skeleton, child_pos, next_radius, color);
        Connect(skeleton, parent_index, radius, child_index, next_radius, color);
        CreateBranches(skeleton, child_index, child_pos, next_length, next_radius, color, level + 1, max_level);
    }
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
