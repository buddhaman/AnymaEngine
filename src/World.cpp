#include "World.h"

void UpdateWorld(World* world)
{
    for(int i = 0; i < world->agents.size; i++)
    {
        Agent* agent = &world->agents[i];
        float dev = 0.04f;
        float speed = 0.1f;
        agent->orientation += GetRandomR32Debug(-dev, dev);
        agent->vel = speed * V2(cosf(agent->orientation), sinf(agent->orientation));
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

    // Eyes
    PushNGon(mesh, agent->pos+dir*eye_d+perp*eye_d, eye_r, eye_sides, eye_orientation, uv, eye_color);
    PushNGon(mesh, agent->pos+dir*eye_d-perp*eye_d, eye_r, eye_sides, eye_orientation, uv, eye_color);

    // Pupils
    PushNGon(mesh, agent->pos+dir*eye_d+perp*eye_d, pupil_r, eye_sides, eye_orientation, uv, pupil_color);
    PushNGon(mesh, agent->pos+dir*eye_d-perp*eye_d, pupil_r, eye_sides, eye_orientation, uv, pupil_color);
}

void RenderWorld(World* world, Mesh2D* mesh, Camera2D* cam)
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

void InitWorld(World* world)
{
    world->size = V2(1000, 1000);
    int nAgents = 64000;
    for(int i = 0; i < nAgents; i++)
    {
        Agent* agent = world->agents.PushBack();
        *agent = {0};
        agent->pos.x = GetRandomR32Debug(0, world->size.x);
        agent->pos.y = GetRandomR32Debug(0, world->size.y);
        agent->type = i < nAgents/2 ? AgentType_Carnivore : AgentType_Herbivore;
        agent->radius = 1.2f;
    }
}
