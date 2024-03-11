#pragma once

#include "AnymUtil.h"
#include "Array.h"

#include "Agent.h"
#include "Mesh2D.h"
#include "Camera2D.h"
#include "Math.h"

struct Chunk
{
    // For convenience
    Vec2 pos;
    int x_idx;
    int y_idx;

    // Index into the world agent array 
    Array<U32> agent_indices;
};

struct World
{
    R32 chunk_size;
    int x_chunks;
    int y_chunks;
    Array<Chunk> chunks;

    Vec2 size;
    Array<Agent> agents;
    Array<U32> visible_agent_indices;
    MemoryArena* arena;

    I32 reproduction_rate;
};

static inline Chunk* 
GetChunk(World* world, int x_idx, int y_idx)
{
    return &world->chunks[x_idx+y_idx*world->x_chunks];
}

static inline Chunk* 
GetChunkAt(World* world, Vec2 at)
{
    int x_chunk = floor(at.x/world->chunk_size);
    int y_chunk = floor(at.y/world->chunk_size);
    x_chunk = Clamp(0, x_chunk, world->x_chunks-1);
    y_chunk = Clamp(0, y_chunk, world->y_chunks-1);
    return GetChunk(world, x_chunk, y_chunk);
}

void 
UpdateWorld(World* world);

void
RenderDetails(Mesh2D* mesh, Agent* agent);

void 
RenderWorld(World* world, Mesh2D* mesh, Camera2D* cam);

EntityID
CastRay(World* world, Ray ray, EntityID exclude_id=0);

void 
RenderDebugInfo(World* world, Mesh2D* mesh, Camera2D* cam);

void
SortAgentsIntoChunks(World* world);

Agent* 
AddAgent(World* world, AgentType type, Vec2 pos);

Agent* 
SelectFromWorld(World* world, Vec2 pos);

void 
InitWorld(World* world);
