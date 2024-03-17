
#include "Agent.h"
#include "World.h"

I32
GetTicksUntilReproduction(World* world, AgentType type)
{
    switch(type)
    {
    case AgentType_Carnivore: return world->carnivore_reproduction_ticks;
    case AgentType_Herbivore: return world->herbivore_reproduction_ticks;
    default: return 0;
    }
}

I32
GetInitialEnergy(World* world, AgentType type)
{
    switch(type)
    {
    case AgentType_Carnivore: return world->carnivore_initial_energy;
    case AgentType_Herbivore: return world->herbivore_initial_energy;
    default: return 0;
    }
}

void
UpdateMovement(World* world, Agent* agent)
{
    R32 turn_speed = 0.04f;
    //agent->orientation += RandomR32Debug(-dev, dev);
    agent->orientation += agent->brain.output[0]*turn_speed;
    R32 speed = agent->brain.output[1]*0.2f;
    Vec2 dir = V2(cosf(agent->orientation), sinf(agent->orientation));
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

}