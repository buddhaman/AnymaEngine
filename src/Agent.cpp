
#include "Agent.h"
#include "World.h"

void UpdateAgent(World* world, Agent* agent)
{
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