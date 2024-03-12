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

struct ChunkCoordinates
{
    U32 x;
    U32 y;
};

static inline ChunkCoordinates
GetChunkCoordinatesFromWorldPos(World* world, Vec2 at)
{ 
    int x_chunk = floor(at.x/world->chunk_size);
    int y_chunk = floor(at.y/world->chunk_size);
    x_chunk = Clamp(0, x_chunk, world->x_chunks-1);
    y_chunk = Clamp(0, y_chunk, world->y_chunks-1);
    return {(U32)x_chunk, (U32)y_chunk};
}

static inline I32
GetXChunk(World* world, R32 x)
{ 
    return floor(x/world->chunk_size); 
}

static inline I32
GetYChunk(World* world, R32 y)
{ 
    return floor(y/world->chunk_size); 
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

// Now this algorithm can double check agents belonging to multiple chunks. Fix this.
static inline Agent* 
CastRay(World* world, Ray ray, R32 ray_length, Agent* exclude_agent=nullptr)
{
    Vec2 at = ray.pos;
    int x_chunk = GetXChunk(world, at.x);
    int y_chunk = GetYChunk(world, at.y);
    if( x_chunk < 0 || 
        y_chunk < 0 || 
        x_chunk >= world->x_chunks || 
        y_chunk >= world->y_chunks) 
    {
        return nullptr;
    }

    R32 traversed = 0.0f;
    int x_dir = at.x < 0.0f ? -1 : 1;
    int y_dir = at.y < 0.0f ? -1 : 1;
    R32 half_chunk = world->chunk_size/2.0f;
    Vec2 intersection;

    while(traversed < ray_length)
    {
        Chunk* chunk = GetChunk(world, x_chunk, y_chunk);
        
        for(U32 agent_idx : chunk->agent_indices)
        {
            Agent* agent = &world->agents[agent_idx];
            if(RayCircleIntersect(ray, {agent->pos, agent->radius}, &intersection))
            {
                Vec
            }
        }

        // Move to next chunk.
        R32 x_edge = (chunk->x_idx + 0.5f)*world->chunk_size + half_chunk*x_dir;
        R32 y_edge = (chunk->y_idx + 0.5f)*world->chunk_size + half_chunk*y_dir;
        R32 x_trav = (x_edge - at.x)/ray.dir.x;
        R32 y_trav = (y_edge - at.y)/ray.dir.y;
        if(x_trav < y_trav)
        {
            traversed += x_trav;
            at = ray.pos + traversed*ray.dir;
            x_chunk += x_dir;
            if(x_chunk < 0 || x_chunk > world->x_chunks) break;
        }
        else
        {
            traversed += y_trav;
            at = ray.pos + traversed*ray.dir;
            y_chunk += y_dir;
            if(y_chunk < 0 || y_chunk > world->y_chunks) break;
        }
    }

    return nullptr;
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
ChunkCollisions(World* world, int center_chunk_x, int center_chunk_y);

void 
InitWorld(World* world);
