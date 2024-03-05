#include "World.h"

void UpdateWorld(World* world)
{
    for(Agent& agent : world->agents)
    {
        UpdateAgent(world, &agent);
    }
}

void RenderWorld(World* world, Mesh2D* mesh)
{
    for(int i = 0; i < world->agents.size; i++)
    {
        Agent* agent = &world->agents[i];
        Vec2 uv = V2(0,0);
        U32 color = 0xff0000ff;
        PushRect(mesh, agent->pos, V2(1,1), uv, uv, color);
    }
}

void InitWorld(World* world)
{
    world->size = V2(100, 100);
    int nAgents = 100;
    for(int i = 0; i < nAgents; i++)
    {
        Agent* agent = world->agents.PushBack();
        *agent = {0};
        agent->pos.x = GetRandomR32Debug(0, world->size.x);
        agent->pos.y = GetRandomR32Debug(0, world->size.y);
    }
}
