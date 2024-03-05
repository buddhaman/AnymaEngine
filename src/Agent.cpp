
#include "Agent.h"

void UpdateAgent(World* world, Agent* agent)
{
    float dev = 0.04f;
    float speed = 0.1f;
    agent->orientation += GetRandomR32Debug(-dev, dev);
    agent->vel = speed * V2(cosf(agent->orientation), sinf(agent->orientation));
    agent->pos += agent->vel;
}