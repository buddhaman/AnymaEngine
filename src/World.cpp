#include "World.h"

// Now this algorithm can double check agents belonging to multiple chunks. Fix this.
// Idea: first collect all agents, sort with pairs where the first index is the length 
Agent* 
CastRay(World* world, Ray ray, R32 ray_length, RayCollision* collision, Agent* exclude_agent)
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
    int x_dir = ray.dir.x < 0.0f ? -1 : 1;
    int y_dir = ray.dir.y < 0.0f ? -1 : 1;
    R32 half_chunk = world->chunk_size/2.0f;

    while(traversed < ray_length)
    {
        Chunk* chunk = GetChunk(world, x_chunk, y_chunk);
        
        // TODO: Make R32_MAX
        R32 min_intersect_dist = 100000.0f;
        Agent* hit = nullptr;
        for(U32 agent_idx : chunk->agent_indices)
        {
            Agent* agent = &world->agents[agent_idx];
            if(agent==exclude_agent) continue;
            if(!RayCircleIntersect(ray, {agent->pos, agent->radius}, collision)) continue;

            if(collision->distance < min_intersect_dist)
            {
                min_intersect_dist = collision->distance;
                hit = agent;
            }
        }

        if(hit)
        {
            return hit;
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
            if(x_chunk < 0 || x_chunk >= world->x_chunks) break;
        }
        else
        {
            traversed += y_trav;
            at = ray.pos + traversed*ray.dir;
            y_chunk += y_dir;
            if(y_chunk < 0 || y_chunk >= world->y_chunks) break;
        }
    }

    return nullptr;
}

static inline bool 
CanAddAgent(World* world, Vec2 pos)
{
    Chunk* chunk = GetChunkAt(world, pos);
    if(chunk->agent_indices.IsFull()) return false;
    return true;
}

static inline void
SuperSimpleBehavior(Agent* agent)
{
        // Behavior
        R32 turnspeed = -0.05f;
        I64 n_eyes = agent->eyes.size;
        if(agent->eyes[0].hit_type)
        {
            if(agent->eyes[0].hit_type==agent->type)
            {
                agent->orientation += turnspeed;
            }
            else
            {
                agent->orientation -= turnspeed;
            }
        }
        if(agent->eyes[n_eyes-1].hit_type)
        {
            if(agent->eyes[n_eyes-1].hit_type==agent->type)
            {
                agent->orientation -= turnspeed;
            }
            else
            {
                agent->orientation += turnspeed;
            }
        }
}

void 
UpdateWorld(World* world)
{
    for(U32 agent_idx : world->active_agents)
    {
        Agent* agent = &world->agents[agent_idx];
#if 1
        RayCollision collision;
        for(int eye_idx = 0; eye_idx < agent->eyes.size; eye_idx++)
        {
            AgentEye* eye = &agent->eyes[eye_idx];
            // Update eye
            Vec2 eye_dir = V2Polar(agent->orientation+eye->orientation, 1.0f);
            eye->ray = {agent->pos, eye_dir};
            eye->distance = 50.0f;
            eye->hit_type = AgentType_None;
            Agent* hit = CastRay(world, eye->ray, eye->distance, &collision, agent);
            if(!hit) { continue; }

            eye->distance = collision.distance;
            eye->hit_type = hit->type;

            agent->brain->input[eye_idx] = eye->hit_type;
        }
        agent->brain->input[agent->brain->input.n-1] = 1.0f;
#endif
        UpdateBrain(agent);
        UpdateMovement(world, agent);

        agent->energy--;
        if(agent->energy <= 0)
        {
            RemoveAgent(world, agent_idx);
        }
        agent->ticks_until_reproduce--;
        if(agent->ticks_until_reproduce <= 0)
        {
            agent->ticks_until_reproduce = GetTicksUntilReproduction(world, agent->type);
            Vec2 at_pos = agent->pos+V2(0.5f, 0.5f);
            int n_rep = RandomR32Debug(0, 1) < 0.5 ? 2 : 1;
            for(int i = 0; i < n_rep; i++)
            {
                if(CanAddAgent(world, at_pos))
                {
                    AddAgent(world, agent->type, at_pos);
                }
            }
        }

        // Update brain
    }

    world->removed_agent_indices.Sort([](U32 a, U32 b) -> int {return b-a;});
    // Note: these are the indices of the indices.
    for(int remove_idx : world->removed_agent_indices)
    {
        world->active_agents.RemoveIndexUnordered(remove_idx);
    }
    world->removed_agent_indices.Clear();

    world->ticks++;
}

void
HerbivoreCarnivoreCollision(Agent* herbivore, Agent* carnivore)
{
    Assert(herbivore->type==AgentType_Herbivore);
    Assert(carnivore->type==AgentType_Carnivore);
    herbivore->energy-=5;
    carnivore->energy+=5;
}

static inline void
CheckAgentCollisions(Agent* agent0, Agent* agent1)
{
    Vec2 diff = agent1->pos - agent0->pos;
    R32 l2 = V2Len2(diff);
    R32 radsum = agent0->radius+agent1->radius;
    if(l2 < radsum*radsum)
    {
        if(agent0->type==AgentType_Carnivore && agent1->type==AgentType_Herbivore)
        {
            HerbivoreCarnivoreCollision(agent1, agent0);
        }
        else if(agent0->type==AgentType_Herbivore && agent1->type==AgentType_Carnivore)
        {
            HerbivoreCarnivoreCollision(agent0, agent1);
        }
        else
        {
            // Resolve collision
            R32 l = sqrtf(l2);
            R32 amount = radsum-l;
            agent1->pos += amount*diff/2.0f;
            agent0->pos -= amount*diff/2.0f;
        }
    }
}

void
RenderEyeRays(Mesh2D* mesh, Agent* agent)
{
    Vec2 uv = V2(0,0);
    for(int i = 0; i < agent->eyes.size; i++)
    {
        AgentEye* eye = &agent->eyes[i];
        R32 ray_width = 0.1f;
        U32 color = GetAgentColor(eye->hit_type);
        PushLine(mesh, eye->ray.pos, eye->ray.pos + eye->distance*eye->ray.dir, ray_width, uv, uv, color);
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
    // Which agents are visible. Omg this is stupid should be done using chunks.
    world->visible_agent_indices.Clear();
    for(U32 agent_idx : world->active_agents)
    {
        Agent* agent = &world->agents[agent_idx];
        if(InBounds({cam->pos-cam->size/2.0f, cam->size}, agent->pos))
        {
            world->visible_agent_indices.PushBack(agent_idx);
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
RenderChunks(World* world, Mesh2D* mesh, Camera2D* cam)
{
    Vec2 chunk_dims = V2(world->chunk_size, world->chunk_size);
    for(int chunk_idx = 0; chunk_idx < world->chunks.size; chunk_idx++)
    {
        Chunk* chunk = &world->chunks[chunk_idx];
        PushLineRect(mesh, chunk->pos, chunk_dims, 1.0f/cam->scale, V2(0,0), V2(0,0), 0x66666666);
    }
}

static inline void
ClearChunks(World* world)
{
    for(int chunk_idx = 0; chunk_idx < world->chunks.size; chunk_idx++)
    {
        world->chunks[chunk_idx].agent_indices.Clear();
    }
}

void
SortAgentsIntoChunks(World* world)
{
    ClearChunks(world);

    for(U32 agent_idx : world->active_agents)
    {
        Agent* agent = &world->agents[agent_idx];
        GetChunkAt(world, agent->pos)->agent_indices.PushBack(agent_idx);
    }
}

void
SortAgentsIntoMultipleChunks(World* world)
{
    ClearChunks(world);

    for(U32 agent_idx : world->active_agents)
    {
        Agent* agent = &world->agents[agent_idx];

        int min_x = Max(0, (int)((agent->pos.x-agent->radius)/world->chunk_size));
        int max_x = Min(world->x_chunks-1, (int)((agent->pos.x+agent->radius)/world->chunk_size));
        int min_y = Max(0, (int)((agent->pos.y-agent->radius)/world->chunk_size));
        int max_y = Min(world->y_chunks-1, (int)((agent->pos.y+agent->radius)/world->chunk_size));
        for(int y = min_y; y <= max_y; y++)
        for(int x = min_x; x <= max_x; x++)
        {
            Chunk* chunk = GetChunk(world, x, y);
            chunk->agent_indices.PushBack(agent_idx);
        }
    }
}

Brain*
GetBrainForAgent(World* world, U32 agent_idx, int n_eyes)
{
    // Already initialized. Not zeroed though.
    Brain* brain = &world->brains[agent_idx];
    if(agent_idx < world->agents.size)
    {
        return brain;
    }

    int inputs = n_eyes+1;
    int outputs = 3;
    brain->gene = VecR32Create(world->arena, inputs*outputs);
    brain->gene.SetNormal(0, 2);
    brain->input = VecR32Create(world->arena, n_eyes+1);
    brain->output = VecR32Create(world->arena, 3);
}

Agent* 
AddAgent(World* world, AgentType type, Vec2 pos, Agent* parent)
{
    Agent* agent = world->agents.PushBack();
    *agent = Agent{};
    agent->pos = pos;
    agent->type = type;
    agent->radius = 1.2f;
    agent->id = 1;          // TODO: Use entity ids
    agent->is_alive = true;

    // Eyes
    int n_eyes = 4;
    R32 agent_fov = 0.6f;
    agent->eyes = CreateArray<AgentEye>(world->arena, n_eyes);
    for(int i = 0; i < n_eyes; i++)
    {
        AgentEye* eye = agent->eyes.PushBack();
        eye->orientation = -agent_fov/2.0f + i*agent_fov/(n_eyes-1);
    }

    // Brain
    //Brain* brain = GetBrainForAgent(world, agent_

    I64 offset = 0;
    brain->weights = brain->gene.ShapeAs(inputs, outputs, offset);

    agent->ticks_until_reproduce = GetTicksUntilReproduction(world, agent->type);
    agent->energy = GetInitialEnergy(world, agent->type);

    return agent;
}

void
RemoveAgent(World* world, U32 agent_idx)
{
    // Later this will be handled differently.
    world->agents[agent_idx].is_alive = false;
    world->removed_agent_indices.PushBack(agent_idx);
}

Agent* 
SelectFromWorld(World* world, Vec2 pos)
{
    Chunk* chunk = GetChunkAt(world, pos);
    for(U32 agent_idx : chunk->agent_indices)
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
            CheckAgentCollisions(a0, a1);
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
            CheckAgentCollisions(a0, a1);
        }
    }
}

World* 
CreateWorld(MemoryArena* arena, SimulationSettings* settings)
{
    World* world = PushStruct(arena, World);
    *world = World{};
    world->arena = arena;
    world->chunk_size = settings->chunk_size;
    world->x_chunks = settings->x_chunks;
    world->y_chunks = settings->y_chunks;
    world->size = world->chunk_size*V2(world->x_chunks, world->y_chunks);

    int max_agents = settings->max_agents;
    int n_initial_agents = settings->n_initial_agents;

    // TODO: This is a heuristic. Do something better.
    int max_agents_in_chunk = (int)(world->chunk_size*world->chunk_size*2);

    world->herbivore_reproduction_ticks = 60*9;
    world->herbivore_initial_energy = 60*10;

    world->carnivore_reproduction_ticks = 60*10;
    world->carnivore_initial_energy = 60*9;

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
    world->active_agents = CreateArray<U32>(world->arena, max_agents);
    world->visible_agent_indices = CreateArray<U32>(world->arena, max_agents);
    world->removed_agent_indices = CreateArray<U32>(world->arena, max_agents);

    for(int i = 0; i < n_initial_agents; i++)
    {
        AgentType type = i < n_initial_agents/2 ? AgentType_Carnivore : AgentType_Herbivore;
        Agent* agent = AddAgent(world, type, RandomVec2Debug(V2(0,0), world->size));
        agent->orientation = RandomR32Debug(-M_PI, M_PI);
    }
    return world;
}
