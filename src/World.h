
#include "AnymUtil.h"
#include "Array.h"

#include "Agent.h"
#include "Mesh2D.h"
#include "Camera2D.h"
#include "Math.h"

struct World
{
    Vec2 size;
    DynamicArray<Agent> agents;
    DynamicArray<U16> visible_agent_indices;
};

void 
UpdateWorld(World* world);

void 
RenderWorld(World* world, Mesh2D* mesh, Camera2D* cam);

EntityID
CastRay(World* world, Ray ray, EntityID exclude_id=0);

void 
InitWorld(World* world);