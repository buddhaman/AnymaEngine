
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

Limb* 
Connect(Skeleton* skeleton, int idx0, R32 width0, int idx1, R32 width1, U32 color)
{
    Limb* limb = skeleton->limbs.PushBack();
    limb->constraint = Connect(skeleton, idx0, idx1);
    limb->from_width = width0;
    limb->to_width = width1;
    limb->color = color;
    return limb;
}

Joint* 
AddJoint(Skeleton* skeleton, Vec3 pos, R32 r, U32 color)
{
    Joint* joint = skeleton->joints.PushBack();
    joint->v = AddParticle(skeleton, pos);
    joint->r = r;
    joint->color = color;
    return joint;
}

void
UpdateSkeleton(Skeleton* skeleton)
{
    float friction = 0.96f;
    for(Verlet3& p : skeleton->particles)
    {
        VerletUpdate(&p, friction);
    }
    for(Verlet3Constraint& c : skeleton->constraints)
    {
        VerletSolve(&c);
    }
}

void
RenderSkeleton(TiltedRenderer* renderer, Skeleton* skeleton)
{
    // Draw outline.
    R32 outline = 0.1f;
    U32 outline_color = Color_Black;

    for(Joint& joint : skeleton->joints)
    {
        RenderCircle(renderer, joint.v->pos, joint.r + outline, outline_color);
    }
    for(Limb& limb : skeleton->limbs)
    {
        RenderTrapezoid(renderer, 
                        limb.constraint->v0->pos, limb.from_width + outline*2, 
                        limb.constraint->v1->pos, limb.to_width + outline*2, 
                        outline_color);
    }

    // Draw actual body.
    for(Limb& limb : skeleton->limbs)
    {
        RenderTrapezoid(renderer, 
                        limb.constraint->v0->pos, limb.from_width, 
                        limb.constraint->v1->pos, limb.to_width, 
                        limb.color);
    }
    for(Joint& joint : skeleton->joints)
    {
        RenderCircle(renderer, joint.v->pos, joint.r, joint.color);
    }
}

// This should eventually be created by a memory pool ? 
Skeleton*
CreateSkeleton(MemoryArena* arena, int max_joints, int max_limbs)
{
    // Scary D:>
    Skeleton* skeleton = PushStruct(arena, Skeleton);
    *skeleton = {0};

    skeleton->particles = CreateArray<Verlet3>(arena, max_joints);
    skeleton->constraints = CreateArray<Verlet3Constraint>(arena, max_limbs);
    skeleton->joints = CreateArray<Joint>(arena, max_joints);
    skeleton->limbs = CreateArray<Limb>(arena, max_limbs);

    #if 0

    U32 color = Color_White;
    R32 r = 0.1f;

    // Head 0
    Joint* v = AddJoint(skeleton, V3(pos.x, pos.y, pos.z + 4.5f*scale), r*4, color);
    // Neck 1
    AddJoint(skeleton, V3(pos.x, pos.y, pos.z + 4.0f*scale), r, color);
    // Waist 2
    AddJoint(skeleton, V3(pos.x, pos.y, pos.z + 2.0f*scale), r, color);
    // Left knee 3 
    AddJoint(skeleton, V3(pos.x, pos.y, pos.z + scale), r, color);
    // Left foot 4
    AddJoint(skeleton, V3(pos.x, pos.y, pos.z), r, color);
    // Right knee 5
    AddJoint(skeleton, V3(pos.x, pos.y, pos.z + scale), r, color);
    // Right foot 6
    AddJoint(skeleton, V3(pos.x, pos.y, pos.z), r, color);
    // Left elbow 7
    AddJoint(skeleton, V3(pos.x, pos.y, pos.z + 3.0f*scale), r, color);
    // Left hand 8
    AddJoint(skeleton, V3(pos.x, pos.y, pos.z + 2.0f*scale), r, color);
    // Right elbow 9
    AddJoint(skeleton, V3(pos.x, pos.y, pos.z + 3.0f*scale), r, color);
    // Right hand 10
    Joint* v2 = AddJoint(skeleton, V3(pos.x, pos.y, pos.z + 2.0f*scale), r, color);

    AddImpulse(v->v, 0.5f*V3(1,1,2));

    Connect(skeleton, 0, r*4, 1, r, color);
    Connect(skeleton, 1, r, 2, r, color);
    Connect(skeleton, 2, r, 3, r, color);
    Connect(skeleton, 3, r, 4, r, color);
    Connect(skeleton, 2, r, 5, r, color);
    Connect(skeleton, 5, r, 6, r, color);
    Connect(skeleton, 1, r, 7, r, color);
    Connect(skeleton, 7, r, 8, r, color);
    Connect(skeleton, 1, r, 9, r, color);
    Connect(skeleton, 9, r, 10, r, color);

    #endif

    return skeleton;
}
