#include "World.h"

EntityID
CastRay(World* world, Ray ray, EntityID exclude_id)
{
    R32 minDist = 10000.0f;
    for(int i = 0; i < world->agents.size; i++)
    {
        Agent* agent = &world->agents[i];
        if(agent->id==exclude_id) continue;
        
        Vec2 intersection;
        RayCircleIntersect(ray, {agent->pos, agent->radius}, &intersection);
        
    }
    return 0;
}

static inline U32
GetAgentColor(AgentType type)
{
    switch(type)
    {
    case AgentType_Carnivore: return 0xff0000ff;
    case AgentType_Herbivore: return 0xff00ff00;
    }
    return 0xffffffff;
}

void 
UpdateWorld(World* world)
{
    for(int i = 0; i < world->agents.size; i++)
    {
        Agent* agent = &world->agents[i];
        R32 dev = 0.04f;
        R32 speed = 0.1f;
        Vec2 dir = V2(cosf(agent->orientation), sinf(agent->orientation));
        agent->orientation += GetRandomR32Debug(-dev, dev);
        agent->vel = speed * dir;
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

        #if 1
        // Update eye
        agent->eye.ray = {agent->pos, dir};
        agent->eye.distance = 50.0f;
        agent->eye.color = 0xffffffff;

        // Cast ray
        for(int j = 0; j < world->agents.size; j++)
        {
            Agent* ac = &world->agents[j];
            if(i==j) continue;
            
            Vec2 intersection;
            if(RayCircleIntersect(agent->eye.ray, {ac->pos, ac->radius}, &intersection))
            {
                R32 dist = V2Dist(intersection, agent->eye.ray.pos);
                if(dist < agent->eye.distance)
                {
                    agent->eye.distance = dist;
                    agent->eye.color = GetAgentColor(ac->type);
                }
            }
        }
        #endif
    }

#if 1
    // Collision detection
    for(int i = 0; i < world->agents.size-1; i++)
    {
        Agent* agent0 = &world->agents[i];
        for(int j = i+1; j < world->agents.size; j++)
        {
            Agent* agent1 = &world->agents[j];
            Vec2 diff = agent1->pos - agent0->pos;
            R32 l2 = V2Len2(diff);
            R32 radsum = agent0->radius+agent1->radius;
            if(l2 < radsum*radsum)
            {
                // Collision
                R32 l = sqrtf(l2);
                R32 amount = radsum-l;
                agent1->pos += amount*diff/2.0f;
                agent0->pos -= amount*diff/2.0f;
            }
        }
    }
#endif
}

static inline void
RenderDetails(Mesh2D* mesh, Agent* agent)
{
    R32 r = agent->radius;
    Vec2 uv = V2(0,0);
    Vec2 dir = V2Polar(agent->orientation, 1.0f);
    Vec2 perp = V2(dir.y, -dir.x);

    // Draw eyes
    I32 eye_sides = 3;
    R32 eye_d = r*0.4f;
    R32 eye_r = r*0.4f;
    R32 eye_orientation = agent->orientation;
    U32 eye_color = 0xffffffff;

    U32 pupil_color = 0xff000000;
    R32 pupil_r = eye_r*0.7f;

    // Draw eye rays
    AgentEye* eye = &agent->eye;
    R32 ray_width = 0.1f;
    PushLine(mesh, eye->ray.pos, eye->ray.pos + eye->distance*eye->ray.direction, ray_width, uv, uv, eye->color);

    // Eyes
    PushNGon(mesh, agent->pos+dir*eye_d+perp*eye_d, eye_r, eye_sides, eye_orientation, uv, eye_color);
    PushNGon(mesh, agent->pos+dir*eye_d-perp*eye_d, eye_r, eye_sides, eye_orientation, uv, eye_color);

    // Pupils
    PushNGon(mesh, agent->pos+dir*eye_d+perp*eye_d, pupil_r, eye_sides, eye_orientation, uv, pupil_color);
    PushNGon(mesh, agent->pos+dir*eye_d-perp*eye_d, pupil_r, eye_sides, eye_orientation, uv, pupil_color);

}

void 
RenderWorld(World* world, Mesh2D* mesh, Camera2D* cam)
{
    for(int i = 0; i < world->agents.size; i++)
    {
        Agent* agent = &world->agents[i];

        R32 r = agent->radius;
        Vec2 uv = V2(0,0);
        Vec2 dir = V2Polar(agent->orientation, 1.0f);
        Vec2 perp = V2(dir.y, -dir.x);
        U32 herbivore_color = 0xff0ff000;
        U32 carnivore_color = 0xff0000ff;

        // TODO: Better just make two completely separate loops
        if(cam->scale > 3)
        {
            if(agent->type==AgentType_Carnivore)
            {
                I32 sides = 3;
                U32 color = carnivore_color;
                PushNGon(mesh, agent->pos, agent->radius, sides, agent->orientation, uv, color);
            }
            else
            {
                I32 sides = 5;
                U32 color = herbivore_color;
                PushNGon(mesh, agent->pos, agent->radius, sides, agent->orientation, uv, color);
            }
            RenderDetails(mesh, agent);
        }
        else
        {
            U32 color = agent->type == AgentType_Carnivore ? carnivore_color : herbivore_color;
            I32 sides = 3;
            PushNGon(mesh, agent->pos, agent->radius, sides, agent->orientation, uv, color);
        }
    }
}

void 
InitWorld(World* world)
{
    world->size = V2(100, 100);
    int nAgents = 50;
    for(int i = 0; i < nAgents; i++)
    {
        Agent* agent = world->agents.PushBack();
        *agent = {0};
        agent->pos.x = GetRandomR32Debug(0, world->size.x);
        agent->pos.y = GetRandomR32Debug(0, world->size.y);
        agent->type = i < nAgents/2 ? AgentType_Carnivore : AgentType_Herbivore;
        agent->radius = 1.2f;
        agent->id = i+1;
    }
}
