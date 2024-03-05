
#include "AnymUtil.h"
#include "Array.h"

#include "Agent.h"
#include "Mesh2D.h"
#include "Camera2D.h"

struct World
{
    Vec2 size;
    DynamicArray<Agent> agents;
    DynamicArray<U16> visible_agent_indices;
};

void UpdateWorld(World* world);
void RenderWorld(World* world, Mesh2D* mesh, Camera2D* cam);
void InitWorld(World* world);