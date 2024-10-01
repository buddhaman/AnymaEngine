
#include "Skeleton.h"

Verlet3* 
AddParticle(Skeleton* skeleton, Vec3 pos)
{
    Verlet3* v = skeleton->particles.PushBack();
    v->pos = v->old_pos = pos;
    return v;
}

Verlet3Constraint* 
Connect(Skeleton* skeleton, int idx0, int idx1)
{
    Verlet3Constraint* constraint = skeleton->constraints.PushBack();
    constraint->v0 = &skeleton->particles[idx0];
    constraint->v1 = &skeleton->particles[idx1];
    constraint->r = V3Len(constraint->v1->pos - constraint->v0->pos);
    return constraint;
}

void
Update(Skeleton* skeleton)
{
    for(Verlet3& p : skeleton->particles)
    {
        VerletUpdate(&p, 0.9f);
    }
    for(Verlet3Constraint& c : skeleton->constraints)
    {
        VerletSolve(&c);
    }
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
    Verlet3* v = AddParticle(skeleton, pos + V3(2,0,1));
    Verlet3* v2 = AddParticle(skeleton, pos + V3(2,2,1));
    AddImpulse(v2, V3(1,1,1));

    Connect(skeleton, 0, 1);
    Connect(skeleton, 1, 2);
    Connect(skeleton, 1, 3);

    return skeleton;
}
