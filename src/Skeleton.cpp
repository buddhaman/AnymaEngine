
#include "Skeleton.h"

void
Update(Skeleton* skeleton)
{
}

void
Render(Renderer* renderer, Skeleton* skeleton)
{
    R32 width = 0.1f;
    for(Verlet3Constraint& constraint : skeleton->constraints)
    {
        RenderLine(renderer, constraint.v0->pos.xy, constraint.v1->pos.xy, width, Color_White);
    }
}

// This should eventually be created by a memory pool ? 
Skeleton*
CreateSkeleton(MemoryArena* arena, Vec3 pos, R32 scale)
{
    // Scary D:>
    Skeleton* skeleton = PushStruct(arena, Skeleton);
    *skeleton = {0};
    return skeleton;
}
