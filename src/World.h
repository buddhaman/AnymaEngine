
#include "AnymUtil.h"
#include "Array.h"

#include "Agent.h"
#include "Mesh2D.h"

struct World
{
    Vec2 size;
    DynamicArray<Agent> agents;
};

void UpdateWorld(World* world);
void RenderWorld(World* world, Mesh2D* mesh);
void InitWorld(World* world);