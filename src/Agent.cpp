
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
    R32 turn_speed = 0.2f;
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

    R32 spec_speed = 0.5f;
    R32 speed = agent->brain->output[2]*spec_speed*(1.0f+charge_amount);
    Vec2 dir = V2Polar(agent->orientation, 1.0f);
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