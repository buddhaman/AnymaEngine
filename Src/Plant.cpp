#include "Plant.h"
#include "World.h"

Plant*
CreatePlant(World* world, Vec2 pos)
{
    MemoryArena* arena = world->plant_arena;
    Plant* plant = PushNewStruct(arena, Plant);
    plant->pos = pos;
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
        // Add some slight randomness to make it more natural
        R32 angle_variation = RandomR32Debug(-10.0f, 10.0f);  
        R32 angle = DegToRad(45 + i * (360.0f / num_branches) + angle_variation); 

        // Spread in the X-Y plane, but add slight Z deviation to avoid linear structures
        Vec3 direction = V3(
            length * cosf(angle), 
            length * sinf(angle),
            length * RandomR32Debug(0.7f, 1.2f)  // Small vertical randomness
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
    Skeleton* skele = plant->skeleton;
    
    R32 gravity = 0.01f; // Going up.
    for (Verlet3& v : skele->particles)
    {
        AddImpulse(&v, V3(0, 0, gravity));
    }

    // Apply repulsion force to spread out leaves
    R32 repulsion_strength = 0.1f;  
    for (int i = 0; i < (int)skele->joints.size-1; i++)
    {
        for (int j = i + 1; j < (int)skele->joints.size; j++)
        {
            Vec3 dir = skele->joints[j].v->pos - skele->joints[i].v->pos;
            R32 dist2 = V3Dot(dir, dir);  

            if (dist2 > 0.0001f)  
            {
                Vec3 repulsion = V3Norm(dir) * (repulsion_strength / dist2);
                AddImpulse(skele->joints[i].v, -repulsion);  
                AddImpulse(skele->joints[j].v, repulsion);
            }
        }
    }

    skele->joints[0].v->pos.xy = plant->pos;
    skele->joints[0].v->pos.z = 0;
    UpdateSkeleton(plant->skeleton);
    skele->joints[0].v->pos.xy = plant->pos;
    skele->joints[0].v->pos.z = 0;
}


void
RenderPlant(TiltedRenderer* renderer, Plant* plant)
{
    RenderSkeleton(renderer, plant->skeleton);
}
