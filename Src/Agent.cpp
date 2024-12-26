
#include "Agent.h"
#include "World.h"

PhenoType* 
CreatePhenotype(MemoryArena* arena, int max_backbones)
{
    PhenoType* body = PushStruct(arena, PhenoType);
    *body = {0};

    body->backbone_radius = VecR32Create(arena, max_backbones);
    body->knee_size = VecR32Create(arena, max_backbones);
    body->foot_size = VecR32Create(arena, max_backbones);
    body->step_radius = VecR32Create(arena, max_backbones);

    body->has_leg = CreateArray<U32>(arena, max_backbones);
    body->has_leg.FillAndSetValue(0);

    body->color = Color_Brown;

    return body;
}

void
InitRandomPhenotype(PhenoType* pheno)
{
    pheno->n_backbones = RandomIntDebug(1, 10);

    // Bettter make differnt distribution, technically this can go below 0.
    pheno->backbone_radius.Set(0);
    pheno->backbone_radius.AddNormal(3.0f, 1.0f);

    pheno->knee_size.Set(0.0f);
    pheno->knee_size.AddNormal(1.0f, 0.5f);

    pheno->foot_size.Set(0.0f);
    pheno->foot_size.AddNormal(1.0f, 0.5f);

    pheno->step_radius.Set(0.0f);
    pheno->step_radius.AddNormal(1.0f, 0.25f);

    for(int i = 0; i < pheno->n_backbones; i++)
    {
        pheno->has_leg[i] = RandomR32Debug(0, 1) < 0.5f;
    }

    pheno->elbow_size = 1.0f;
    pheno->hand_size = 1.0f;

    pheno->color = Vec4ToColor(V4(
        RandomR32Debug(0.0f, 1.0f),
        RandomR32Debug(0.0f, 1.0f),
        RandomR32Debug(0.0f, 1.0f),
        1.0f
    ));
}

static inline Leg* 
InitLeg(Agent* agent, Leg* leg, int idx_in_body, int dir, R32 mid_radius, R32 end_radius, U32 color)
{
    Skeleton* skele = agent->skeleton;
    int body_particle_idx = agent->body.body[idx_in_body];
    Joint* attach_to = &skele->joints[body_particle_idx];
    Vec3 ground_diff = V3(0,0,attach_to->v->pos.z);
    R32 back_size = agent->phenotype->backbone_radius[idx_in_body];
    AddJoint(skele, attach_to->v->pos-0.5f*ground_diff, mid_radius, color);
    AddJoint(skele, attach_to->v->pos-1.0f*ground_diff, end_radius, color);

    // set idx to foot.
    leg->idx = (int)skele->joints.size-1;
    leg->target_offset.xy = agent->body.target_positions[idx_in_body].xy;
    leg->target_offset.y += dir*3;
    leg->target_offset.z = 0.0f;
    leg->r = agent->phenotype->step_radius[idx_in_body];

    Connect(skele, body_particle_idx, back_size*2, leg->idx-1, mid_radius*2, color);
    Connect(skele, leg->idx-1, mid_radius*2, leg->idx, end_radius*2, color);

    return leg;
}

void 
InitAgentSkeleton(MemoryArena* arena, Agent* agent)
{
    PhenoType* phenotype = agent->phenotype;
    Skeleton* skele = agent->skeleton;

    int n_backbones = phenotype->n_backbones;
    U32 color = phenotype->color;
    R32 scale = 1.0f;

    agent->legs = CreateArray<Leg>(arena, n_backbones*2);

    // Start with neck
    Vec3 pos = V3(0,0,scale*8);

    // Body
    R32 diff = 9.0f*scale;

    // Offset beginning position by diff*(n_backbones-1)/2 such that the agent is centered.
    pos -= V3((n_backbones-1)*diff/2.0f, 0, 0);

    agent->body.body = CreateArray<int>(arena, n_backbones);
    agent->body.target_positions = CreateArray<Vec3>(arena, n_backbones);
    for(int i = 0; i < n_backbones; i++)
    {
        R32 r1 = phenotype->backbone_radius[i];
        AddJoint(skele, pos, r1, color);
        int idx = (int)skele->particles.size-1;
        agent->body.body.PushBack(idx);
        agent->body.target_positions.PushBack(pos);

        pos += i < n_backbones-1 ? V3(diff, 0, 0) : V3(0,0,0);

        if(idx==0) continue;

        int prev_idx = idx-1;
        R32 r0 = phenotype->backbone_radius[prev_idx];
        Connect(skele, prev_idx, r0*2, i, r1*2, color);
    }

    for(int i = 0; i < n_backbones; i++)
    {
        if(!phenotype->has_leg[i]) continue;
        R32 knee_size = agent->phenotype->knee_size[i];
        R32 foot_size = agent->phenotype->foot_size[i];
        InitLeg(agent, agent->legs.PushBack(), i, -1, knee_size, foot_size, color);
        InitLeg(agent, agent->legs.PushBack(), i, +1, knee_size, foot_size, color);
    }

    InitLeg(agent, &agent->l_arm, n_backbones-1, -1, phenotype->elbow_size, phenotype->hand_size, color);
    InitLeg(agent, &agent->r_arm, n_backbones-1, -1, phenotype->elbow_size, phenotype->hand_size, color);

    // Head
    pos += V3(0,0,diff);
    agent->head.target_position = pos;
    AddJoint(skele, agent->head.target_position, scale*2, color);
    agent->head.idx = (int)skele->joints.size-1;

    // Connect head to last bodypart in body
    int last_idx = agent->body.body.Last();
    Connect(skele, last_idx, scale, agent->head.idx, scale, color);

    // Transform the entire skeleton to the current agent position and reset the speeds.
    Vec3 center = V3(agent->pos.x, agent->pos.y, 0.0f);
    Vec2 forward = V2Polar(agent->orientation, 1.0f);
    for(Verlet3& v : skele->particles)
    {
        v.pos = XForm(center, forward, v.pos);
        v.old_pos = v.pos;
    }
}

void
UpdateAgentSkeleton(Agent* agent)
{
    Vec3 direction = V3(0,0,0);
    direction.xy = V2Polar(agent->orientation, 1.0f);
    Vec3 perp = V3(-direction.y, direction.x, 0);

    Vec3 center;
    center.xy = agent->pos;
    center.z = 0.0f;

    MainBody* body = &agent->body;
    Skeleton* skeleton = agent->skeleton;

    R32 body_force = (agent->legs.size==0) ? -0.4f : 0.4f;
    for(int bodypart_idx : body->body)
    {
        Joint* j = &skeleton->joints[bodypart_idx];
        AddImpulse(j->v, V3(0,0,body_force));
    }

    for(Verlet3& p : skeleton->particles)
    {
        if(p.pos.z < 0.0f) p.pos.z = 0.0f;
    }

    for(Leg& leg : agent->legs)
    {
        Joint* j = &skeleton->joints[leg.idx];
        
        Vec3 world_target = XForm(center, direction.xy, leg.target_offset);

        // distance of foot to target. If more than r then move foot.
        R32 feet_diff = V3Len(world_target - leg.foot_pos);
        if(feet_diff > leg.r)
        {
            leg.foot_pos = world_target;
            leg.foot_pos.xy += direction.xy * RandomR32Debug(0.0f, leg.r);
        }

        j->v->pos.z = 0;
        j->v->pos = leg.foot_pos;
    }

    // Update arms
    Joint* l_arm = &skeleton->joints[agent->l_arm.idx];
    AddImpulse(l_arm->v, perp*0.5f);
    Joint* r_arm = &skeleton->joints[agent->r_arm.idx];
    AddImpulse(r_arm->v, -perp*0.5f);

    // Move head up and set X,Y to proper agent x, y position
    Joint* head = &skeleton->joints[agent->head.idx];

    // Only takes into account agents head X position. But that should work.
    head->v->pos.xy = agent->pos + V2Polar(agent->orientation, agent->head.target_position.x);

    AddImpulse(head->v, V3(0,0,body_force));
    UpdateSkeleton(agent->skeleton);
}

I32
GetTicksUntilReproduction(World* world, AgentType type)
{
    switch(type)
    {
    case AgentType_Carnivore: return global_settings.carnivore_reproduction_ticks;
    case AgentType_Herbivore: return global_settings.herbivore_reproduction_ticks;
    default: return 0;
    }
}

I32
GetInitialEnergy(World* world, AgentType type)
{
    switch(type)
    {
    case AgentType_Carnivore: return global_settings.carnivore_initial_energy;
    case AgentType_Herbivore: return global_settings.herbivore_initial_energy;
    default: return 0;
    }
}

void
UpdateMovement(World* world, Agent* agent)
{
    R32 turn_speed = 0.05f;
    agent->orientation += (agent->brain->output[0]-agent->brain->output[1])*turn_speed;
    R32 charge_threshold = 0.25f;
    R32 charge_output = agent->brain->output[3];

    R32 charge_amount = 0;

    // Charging is currently only for carnivores.
    if(agent->type == AgentType_Carnivore)
    {
        if(!agent->charge_refractory && charge_output > charge_threshold)
        {
            // Start charging.
            agent->charge = global_settings.charge_duration;
            agent->charge_refractory = global_settings.charge_refractory_period + global_settings.charge_duration;
            agent->energy -= 40;
        }

        if(agent->charge)
        {
            agent->charge--;
            charge_amount = 1.0;
        }

        if(agent->charge_refractory)
        {
            agent->charge_refractory--;
        }
    }

    R32 spec_speed = 0.25f;
    R32 friction = 0.3f;
    R32 speed = agent->brain->output[2]*spec_speed*(1.0f+charge_amount);
    Vec2 dir = V2Polar(agent->orientation, 1.0f);
    Vec2 vel = speed * dir;
    agent->vel *= friction;
    agent->vel += vel;
    agent->pos += agent->vel;

    Vec2 offset = V2(0,0);
    Vec2 size = world->size;

    if(agent->pos.x < offset.x)
    {
        agent->pos.x = agent->pos.x + size.x;
    }
    
    if(agent->pos.x > offset.x + size.x)
    {
        agent->pos.x = agent->pos.x - size.x;
    }

    if(agent->pos.y < offset.y)
    {
        agent->pos.y = agent->pos.y + size.y;
    }
    
    if(agent->pos.y > offset.y + size.y)
    {
        agent->pos.y = agent->pos.y - size.y;
    }

}