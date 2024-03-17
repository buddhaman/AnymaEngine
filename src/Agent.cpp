
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
