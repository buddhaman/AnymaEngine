#include "World.h"

static inline U32
GetAgentColor(AgentType type)
{
    switch(type)
    {
    case AgentType_Carnivore: return 0xff0000ff;
    case AgentType_Herbivore: return 0xff00ff00;
    }
    return 0xffffffff;
}

void 
UpdateWorld(World* world)
{
    for(int agent_idx = 0; agent_idx < world->agents.size; agent_idx++)
    {
        Agent* agent = &world->agents[agent_idx];
        R32 dev = 0.04f;
        R32 speed = 0.1f;
        Vec2 dir = V2(cosf(agent->orientation), sinf(agent->orientation));
        agent->orientation += GetRandomR32Debug(-dev, dev);
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

        agent->ticks_until_reproduce--;
        if(agent->ticks_until_reproduce <= 0)
        {
            agent->ticks_until_reproduce = world->reproduction_rate;
            if(world->agents.size < world->agents.capacity)
            {
                AddAgent(world, agent->type, agent->pos+V2(0.5f, 0.5f));
            }
        }
#if 0
        for(int eye_idx = 0; eye_idx < agent->eyes.size; eye_idx++)
        {
            AgentEye* eye = &agent->eyes[eye_idx];
            // Update eye
            Vec2 eye_dir = V2Polar(agent->orientation+eye->orientation, 1.0f);
            eye->ray = {agent->pos, eye_dir};
            eye->distance = 50.0f;
            eye->color = 0xffffffff;

            // Cast ray
            for(int raycheck_agent_idx = 0; raycheck_agent_idx < world->agents.size; raycheck_agent_idx++)
            {
               Agent* ac = &world->agents[raycheck_agent_idx];
               if(agent_idx==raycheck_agent_idx) continue;
                
                Vec2 intersection;
                if(RayCircleIntersect(eye->ray, {ac->pos, ac->radius}, &intersection))
                {
                    R32 dist = V2Dist(intersection, eye->ray.pos);
                    if(dist < eye->distance)
                    {
                        eye->distance = dist;
                        eye->color = GetAgentColor(ac->type);
                    }
                }
            }
        }
#endif

    }

}

static inline void
CheckCollisions(Agent* agent0, Agent* agent1)
{
    Vec2 diff = agent1->pos - agent0->pos;
    R32 l2 = V2Len2(diff);
    R32 radsum = agent0->radius+agent1->radius;
    if(l2 < radsum*radsum)
    {
        // Collision
        R32 l = sqrtf(l2);
        R32 amount = radsum-l;
        agent1->pos += amount*diff/2.0f;
        agent0->pos -= amount*diff/2.0f;
    }
}


void
RenderDetails(Mesh2D* mesh, Agent* agent)
{
    R32 r = agent->radius;
    Vec2 uv = V2(0,0);
    Vec2 dir = V2Polar(agent->orientation, 1.0f);
    Vec2 perp = V2(dir.y, -dir.x);

    // Draw eyes
    I32 eye_sides = 3;
    R32 eye_d = r*0.4f;
    R32 eye_r = r*0.4f;
    R32 eye_orientation = agent->orientation;
    U32 eye_color = 0xffffffff;

    U32 pupil_color = 0xff000000;
    R32 pupil_r = eye_r*0.7f;

#if 1
    // Draw eye rays
    for(int i = 0; i < agent->eyes.size; i++)
    {
        AgentEye* eye = &agent->eyes[i];
        R32 ray_width = 0.1f;
        PushLine(mesh, eye->ray.pos, eye->ray.pos + eye->distance*eye->ray.dir, ray_width, uv, uv, eye->color);
    }
#endif

    // Eyes
    PushNGon(mesh, agent->pos+dir*eye_d+perp*eye_d, eye_r, eye_sides, eye_orientation, uv, eye_color);
    PushNGon(mesh, agent->pos+dir*eye_d-perp*eye_d, eye_r, eye_sides, eye_orientation, uv, eye_color);

    // Pupils
    PushNGon(mesh, agent->pos+dir*eye_d+perp*eye_d, pupil_r, eye_sides, eye_orientation, uv, pupil_color);
    PushNGon(mesh, agent->pos+dir*eye_d-perp*eye_d, pupil_r, eye_sides, eye_orientation, uv, pupil_color);
}

void 
RenderWorld(World* world, Mesh2D* mesh, Camera2D* cam)
{
    // Which agents are visible.
    world->visible_agent_indices.Clear();
    for(U32 i = 0; i < world->agents.size; i++)
    {
        Agent* agent = &world->agents[i];
        if(InBounds({cam->pos-cam->size/2.0f, cam->size}, agent->pos))
        {
            world->visible_agent_indices.PushBack(i);
        }
    }

    for(int i = 0; i < world->visible_agent_indices.size; i++)
    {
        Agent* agent = &world->agents[world->visible_agent_indices[i]];

        R32 r = agent->radius;
        Vec2 uv = V2(0,0);
        Vec2 dir = V2Polar(agent->orientation, 1.0f);
        Vec2 perp = V2(dir.y, -dir.x);
        U32 herbivore_color = 0xff0ff000;
        U32 carnivore_color = 0xff0000ff;
        // TODO: Better just make two completely separate loops
        if(cam->scale > 3)
        {
            if(agent->type==AgentType_Carnivore)
            {
                I32 sides = 3;
                U32 color = carnivore_color;
                PushNGon(mesh, agent->pos, agent->radius, sides, agent->orientation, uv, color);
            }
            else
            {
                I32 sides = 5;
                U32 color = herbivore_color;
                PushNGon(mesh, agent->pos, agent->radius, sides, agent->orientation, uv, color);
            }
            RenderDetails(mesh, agent);
        }
        else
        {
            U32 color = agent->type == AgentType_Carnivore ? carnivore_color : herbivore_color;
            I32 sides = 3;
            PushNGon(mesh, agent->pos, agent->radius, sides, agent->orientation, uv, color);
        }
    }
}

void 
RenderDebugInfo(World* world, Mesh2D* mesh, Camera2D* cam)
{
    Vec2 chunk_dims = V2(world->chunk_size, world->chunk_size);
    for(int chunk_idx = 0; chunk_idx < world->chunks.size; chunk_idx++)
    {
        Chunk* chunk = &world->chunks[chunk_idx];
        PushLineRect(mesh, chunk->pos, chunk_dims, 1.0f/cam->scale, V2(0,0), V2(0,0), 0x88888888);
    }
}

void
SortAgentsIntoChunks(World* world)
{
    for(int chunk_idx = 0; chunk_idx < world->chunks.size; chunk_idx++)
    {
        world->chunks[chunk_idx].agent_indices.Clear();
    }

    for(U32 agent_idx = 0; agent_idx < world->agents.size; agent_idx++)
    {
        Agent* agent = &world->agents[agent_idx];
        GetChunkAt(world, agent->pos)->agent_indices.PushBack(agent_idx);
    }
}

Agent* 
AddAgent(World* world, AgentType type, Vec2 pos)
{
    Agent* agent = world->agents.PushBack();
    *agent = Agent{};
    agent->pos = pos;
    agent->type = type;
    agent->radius = 1.2f;
    agent->id = 1;          // TODO: Use entity ids

    int n_eyes = 4;
    R32 agent_fov = 0.8f;
    agent->eyes = CreateArray<AgentEye>(world->arena, n_eyes);
    for(int i = 0; i < n_eyes; i++)
    {
        AgentEye* eye = agent->eyes.PushBack();
        eye->orientation = -agent_fov/2.0f + i*agent_fov/(n_eyes-1);
    }

    agent->ticks_until_reproduce = world->reproduction_rate;

    return agent;
}

Agent* 
SelectFromWorld(World* world, Vec2 pos)
{
    Chunk* chunk = GetChunkAt(world, pos);
    for(int agent_idx = 0; agent_idx < chunk->agent_indices.size; agent_idx++)
    {
        Agent* agent = &world->agents[chunk->agent_indices[agent_idx]];
        R32 r2 = V2Dist2(agent->pos, pos);
        if(r2 < agent->radius)
        {
            return agent;
        }
    }
    return nullptr;
}

static inline void 
ChunkCollisions(World* world, Chunk* chunk0, Chunk* chunk1)
{
    for(int i = 0; i < chunk0->agent_indices.size; i++)
    {
        U32 a0_idx = chunk0->agent_indices[i];
        Agent* a0 = &world->agents[a0_idx];

        for(int j = 0; j < chunk1->agent_indices.size; j++)
        {
            U32 a1_idx = chunk1->agent_indices[j];
            Agent* a1 = &world->agents[a1_idx];
            CheckCollisions(a0, a1);
        }
    }
}

void
ChunkCollisions(World* world, int center_chunk_x, int center_chunk_y)
{
    // First try, naive algorithm.
    Chunk* center_chunk = GetChunk(world, center_chunk_x, center_chunk_y);

    bool has_right = center_chunk_x < world->x_chunks-1;
    bool has_down = center_chunk_y < world->y_chunks-1;
    if(has_right) 
    {
        ChunkCollisions(world, center_chunk, GetChunk(world, center_chunk_x+1, center_chunk_y));
    }
    if(has_down) 
    {
        ChunkCollisions(world, center_chunk, GetChunk(world, center_chunk_x, center_chunk_y+1));
    }
    if(has_down && has_right)
    {
        ChunkCollisions(world, center_chunk, GetChunk(world, center_chunk_x+1, center_chunk_y+1));
    }

    // Inside center chunk.
    for(int i = 0; i < center_chunk->agent_indices.size-1; i++)
    {
        U32 a0_idx = center_chunk->agent_indices[i];
        Agent* a0 = &world->agents[a0_idx];

        for(int j = i+1; j < center_chunk->agent_indices.size; j++)
        {
            U32 a1_idx = center_chunk->agent_indices[j];
            Agent* a1 = &world->agents[a1_idx];
            CheckCollisions(a0, a1);
        }
    }
}

void 
InitWorld(World* world)
{
    world->arena = CreateMemoryArena(MegaBytes(512));
    world->chunk_size = 10;
    world->x_chunks = 200;
    world->y_chunks = 200;
    world->size = world->chunk_size*V2(world->x_chunks, world->y_chunks);

    int max_agents = 128000;
    int n_initial_agents = 200;

    // TODO: This is a heuristic. Do something better.
    int max_agents_in_chunk = (int)(world->chunk_size*world->chunk_size*2);

    world->reproduction_rate = 60;

    world->chunks = CreateArray<Chunk>(world->arena, world->x_chunks*world->y_chunks);
    for(int y = 0; y < world->y_chunks; y++)
    for(int x = 0; x < world->x_chunks; x++)
    {
        Chunk* chunk = world->chunks.PushBack();
        chunk->pos = V2(x*world->chunk_size, y*world->chunk_size);
        chunk->x_idx = x;
        chunk->y_idx = y;
        chunk->agent_indices = CreateArray<U32>(world->arena, max_agents_in_chunk);
    }

    world->agents = CreateArray<Agent>(world->arena, max_agents);
    world->visible_agent_indices = CreateArray<U32>(world->arena, max_agents);

    for(int i = 0; i < n_initial_agents; i++)
    {
        AgentType type = i < n_initial_agents/2 ? AgentType_Carnivore : AgentType_Herbivore;
        Agent* agent = AddAgent(world, type, GetRandomVec2Debug(V2(0,0), world->size));
    }
}
