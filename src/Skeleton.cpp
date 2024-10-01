
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
UpdateSkeleton(Skeleton* skeleton)
{
    for(Verlet3& p : skeleton->particles)
    {
        VerletUpdate(&p, 0.97f);
    }
    for(Verlet3Constraint& c : skeleton->constraints)
    {
        VerletSolve(&c);
    }
}

void
RenderSkeleton(Renderer* renderer, Skeleton* skeleton)
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
    skeleton->constraints = CreateArray<Verlet3Constraint>(arena, 20);

    // Head 0
    Verlet3* v = AddParticle(skeleton, V3(pos.x, pos.y, pos.z + 4.5f*scale));
    // Neck 1
    AddParticle(skeleton, V3(pos.x, pos.y, pos.z + 4.0f*scale));
    // Waist 2
    AddParticle(skeleton, V3(pos.x, pos.y, pos.z + 2.0f*scale));
    // Left knee 3 
    AddParticle(skeleton, V3(pos.x, pos.y, pos.z + scale));
    // Left foot 4
    AddParticle(skeleton, V3(pos.x, pos.y, pos.z));
    // Right knee 5
    AddParticle(skeleton, V3(pos.x, pos.y, pos.z + scale));
    // Right foot 6
    AddParticle(skeleton, V3(pos.x, pos.y, pos.z));
    // Left elbow 7
    AddParticle(skeleton, V3(pos.x, pos.y, pos.z + 3.0f*scale));
    // Left hand 8
    AddParticle(skeleton, V3(pos.x, pos.y, pos.z + 2.0f*scale));
    // Right elbow 9
    AddParticle(skeleton, V3(pos.x, pos.y, pos.z + 3.0f*scale));
    // Right hand 10
    Verlet3* v2 = AddParticle(skeleton, V3(pos.x, pos.y, pos.z + 2.0f*scale));

    AddImpulse(v, V3(1,1,1));
    AddImpulse(v2, V3(-1,-5,-1));

    Connect(skeleton, 0, 1);
    Connect(skeleton, 1, 2);
    Connect(skeleton, 2, 3);
    Connect(skeleton, 3, 4);
    Connect(skeleton, 2, 5);
    Connect(skeleton, 5, 6);
    Connect(skeleton, 1, 7);
    Connect(skeleton, 7, 8);
    Connect(skeleton, 1, 9);
    Connect(skeleton, 9, 10);

    return skeleton;
}
