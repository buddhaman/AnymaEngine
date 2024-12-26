#pragma once

#include "AnymUtil.h"
#include "Array.h"

#include "Agent.h"
#include "Plant.h"

#include "Mesh2D.h"
#include "Camera2D.h"
#include "TMath.h"
#include "SimulationSettings.h"
#include "ParticleSystem.h"

enum ColorOverlay
{
    ColorOverlay_AgentType,
    ColorOverlay_AgentGenes
};

enum CellType
{
    CellType_Grass,
    CellType_Sand,
};

U32
GetCellTypeColor(CellType type)
{
    switch(type)
    {
    case CellType_Grass: return HexToColor(0x4caf50ff); 
    case CellType_Sand: return HexToColor(0xf4d03fff);
    }

    // Invalid type
    return Color_Pink;
}

struct Cell
{
    // For convenience
    Vec2 pos;
    int x_idx;
    int y_idx;
    CellType type;

    // Index into the world agent array 
    Array<U32> agent_indices;
};

struct World
{
    R32 cell_size;
    int x_cells;
    int y_cells;
    Array<Cell> cells;
    Array<U32> cell_corner_colors;

    Vec2 size;

    Array<Entity*> entities;
    Array<Agent*> agents;
    Array<Plant*> plants;

    // Tracking for dynamic entities.
    Array<U32> removed_agent_indices;
    Array<U32> visible_agent_indices;
    int max_eyes_per_agent;

    I64 ticks;

    I32 num_agenttype[AgentType_Num];

    MemoryArena* arena;
    MemoryArena* lifespan_arena;
    MemoryArena* lifespan_arena_old;
    I64 lifespan_arena_swap_ticks;
    I64 max_lifespan;

    bool spawn_particles;
    Array<Particle> particles;
};

static inline Cell* 
GetCell(World* world, int x_idx, int y_idx)
{
    return &world->cells[x_idx+y_idx*world->x_cells];
}

struct CellCoordinates
{
    U32 x;
    U32 y;
};

static inline CellCoordinates
GetCellCoordinatesFromWorldPos(World* world, Vec2 at)
{ 
    int x_cell = (int)(at.x/world->cell_size);
    int y_cell = (int)(at.y/world->cell_size);
    x_cell = Clamp(0, x_cell, world->x_cells-1);
    y_cell = Clamp(0, y_cell, world->y_cells-1);
    return {(U32)x_cell, (U32)y_cell};
}

static inline I32
GetXCell(World* world, R32 x)
{ 
    return (int)(x/world->cell_size); 
}

static inline I32
GetYCell(World* world, R32 y)
{ 
    return (int)(y/world->cell_size); 
}

static inline Cell* 
GetCellAt(World* world, Vec2 at)
{
    int x_cell = (int)(at.x/world->cell_size);
    int y_cell = (int)(at.y/world->cell_size);
    x_cell = Clamp(0, x_cell, world->x_cells-1);
    y_cell = Clamp(0, y_cell, world->y_cells-1);
    return GetCell(world, x_cell, y_cell);
}

Agent* 
CastRay(World* world, Ray ray, R32 ray_length, RayCollision *collision, Agent* exclude_agent=nullptr);

bool 
CanAddAgent(World* world, Vec2 pos);

void 
UpdateWorld(World* world);

void
RenderEyeRays(Mesh2D* mesh, Agent* agent, Vec2 uv);

void
RenderAgent(TiltedRenderer* renderer, Agent* agent);

void
RenderDetails(Mesh2D* mesh, Agent* agent, Vec2 uv);

void 
RenderHealth(Mesh2D* mesh, Agent* agent, Vec2 uv);

void 
RenderWorld(World* world, Mesh2D* mesh, Camera2D* cam, Vec2 uv, ColorOverlay color_overlay = ColorOverlay_AgentType);

void 
DrawCells(World* world, Mesh2D* mesh, Camera2D* cam, Vec2 uv);

void
SortAgentsIntoMultipleCells(World* world);

void
UpdateAgentSensorsAndBrains(World* world, I32 from_idx, I32 to_idx);

void
UpdateAgentBehavior(Camera2D* cam, World* world, I32 from_idx, I32 to_idx);

void
UpdateWorldChanges(World* world);

Agent* 
AddAgent(World* world, AgentType type, Vec2 pos, Agent* parent=nullptr);

void
RemoveAgent(World* world, U32 agent_idx);

Agent* 
SelectFromWorld(World* world, Vec2 pos, R32 extra_radius = 0.0f);

void
CellCollisions(World* world, int center_cell_x, int center_cell_y);

Particle* 
TrySpawnParticle(World* world, Vec2 pos, Vec2 vel, R32 lifetime, U32 color);

void
UpdateParticles(World* world);

World*
CreateWorld(MemoryArena* arena);

