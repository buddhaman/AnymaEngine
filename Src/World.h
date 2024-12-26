#pragma once

#include "AnymUtil.h"
#include "Array.h"

#include "Agent.h"

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

enum ChunkType
{
    ChunkType_Grass,
    ChunkType_Sand,
};

U32
GetChunkTypeColor(ChunkType type)
{
    switch(type)
    {
    case ChunkType_Grass: return HexToColor(0x4caf50ff); 
    case ChunkType_Sand: return HexToColor(0xf4d03fff);
    }

    // Invalid type
    return Color_Pink;
}

struct Chunk
{
    // For convenience
    Vec2 pos;
    int x_idx;
    int y_idx;
    ChunkType type;

    // Index into the world agent array 
    Array<U32> agent_indices;
};

struct World
{
    R32 chunk_size;
    int x_chunks;
    int y_chunks;
    Array<Chunk> chunks;
    Array<U32> chunk_corner_colors;

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

static inline Chunk* 
GetChunk(World* world, int x_idx, int y_idx)
{
    return &world->chunks[x_idx+y_idx*world->x_chunks];
}

struct ChunkCoordinates
{
    U32 x;
    U32 y;
};

static inline ChunkCoordinates
GetChunkCoordinatesFromWorldPos(World* world, Vec2 at)
{ 
    int x_chunk = (int)(at.x/world->chunk_size);
    int y_chunk = (int)(at.y/world->chunk_size);
    x_chunk = Clamp(0, x_chunk, world->x_chunks-1);
    y_chunk = Clamp(0, y_chunk, world->y_chunks-1);
    return {(U32)x_chunk, (U32)y_chunk};
}

static inline I32
GetXChunk(World* world, R32 x)
{ 
    return (int)(x/world->chunk_size); 
}

static inline I32
GetYChunk(World* world, R32 y)
{ 
    return (int)(y/world->chunk_size); 
}

static inline Chunk* 
GetChunkAt(World* world, Vec2 at)
{
    int x_chunk = (int)(at.x/world->chunk_size);
    int y_chunk = (int)(at.y/world->chunk_size);
    x_chunk = Clamp(0, x_chunk, world->x_chunks-1);
    y_chunk = Clamp(0, y_chunk, world->y_chunks-1);
    return GetChunk(world, x_chunk, y_chunk);
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
RenderDetails(Mesh2D* mesh, Agent* agent, Vec2 uv);

void 
RenderHealth(Mesh2D* mesh, Agent* agent, Vec2 uv);

void 
RenderWorld(World* world, Mesh2D* mesh, Camera2D* cam, Vec2 uv, ColorOverlay color_overlay = ColorOverlay_AgentType);

void 
DrawChunks(World* world, Mesh2D* mesh, Camera2D* cam, Vec2 uv);

void
SortAgentsIntoMultipleChunks(World* world);

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
ChunkCollisions(World* world, int center_chunk_x, int center_chunk_y);

Particle* 
TrySpawnParticle(World* world, Vec2 pos, Vec2 vel, R32 lifetime, U32 color);

void
UpdateParticles(World* world);

World*
CreateWorld(MemoryArena* arena);

