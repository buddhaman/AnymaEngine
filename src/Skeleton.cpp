
#include "Skeleton.h"

void
AddParticle(Skeleton* skeleton, Vec3 pos)
{
    Verlet3* v = skeleton->particles.PushBack();
    v->pos = v->old_pos = pos;
}

void
Connect(Skeleton* skeleton, int idx0, int idx1)
{
    Verlet3Constraint* constraint = skeleton->constraints.PushBack();
    constraint->v0 = &skeleton->particles[idx0];
    constraint->v1 = &skeleton->particles[idx1];
    constraint->r = V3Len(constraint->v1->pos - constraint->v0->pos);
}

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

    skeleton->particles = CreateArray<Verlet3>(arena, 12);
    skeleton->constraints = CreateArray<Verlet3Constraint>(arena, 144);

    AddParticle(skeleton, pos);
    AddParticle(skeleton, pos + V3(1,1,1));
    AddParticle(skeleton, pos + V3(2,0,1));

    Connect(skeleton, 0, 1);
    Connect(skeleton, 1, 2);

    return skeleton;
}
