#include "World.h"

static void
SwapLifespanArenas(World* world)
{
    // TODO: Dont clear to zero by default. TODO: Maybe in debug mode.
    ZeroArena(world->lifespan_arena_old);
    ClearArena(world->lifespan_arena_old);
    MemoryArena* tmp = world->lifespan_arena;
    world->lifespan_arena = world->lifespan_arena_old;
    world->lifespan_arena_old = tmp;
}

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
        
        R32 min_intersect_dist = R32_MAX;
        Agent* hit = nullptr;
        for(U32 agent_idx : chunk->agent_indices)
        {
            Agent* agent = world->agents[agent_idx];
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

bool 
CanAddAgent(World* world, Vec2 pos)
{
    Chunk* chunk = GetChunkAt(world, pos);
    if(world->agents.IsFull()) return false;
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
UpdateAgentSensorsAndBrains(World* world, I32 from_idx, I32 to_idx)
{
    for(I32 agent_idx = from_idx; agent_idx < to_idx; agent_idx++)
    {
        Agent* agent = world->agents[agent_idx];
        RayCollision collision;
        agent->brain->input.Set(0);

        // Raycasting and updating eye sensors.
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
            if(eye->hit_type != 0)
            {
                agent->brain->input[eye_idx*3+eye->hit_type-1] = 1.0f;
                agent->brain->input[eye_idx*3+2] = 1.2f*(1.0f - eye->distance/50.0f);
            }
        }
        agent->brain->input[agent->brain->input.n-1] = 1.0f;

        UpdateBrain(agent);
    }
}

void
UpdateAgentBehavior(World* world, I32 from_idx, I32 to_idx)
{
    for(I32 agent_idx = from_idx; agent_idx < to_idx; agent_idx++)
    {
        Agent* agent = world->agents[agent_idx];
        UpdateMovement(world, agent);

        agent->energy--;
        agent->ticks_alive++;
        if(agent->energy <= 0 || agent->ticks_alive >= world->max_lifespan)
        {
            RemoveAgent(world, agent_idx);
        }
        agent->ticks_until_reproduce--;
        if(agent->ticks_until_reproduce <= 0)
        {
            agent->ticks_until_reproduce = GetTicksUntilReproduction(world, agent->type);
            Vec2 at_pos = agent->pos+V2(0.5f, 0.5f);
            int more = agent->type==AgentType_Carnivore ? 4 : 2;
            int n_rep = RandomR32Debug(0, 1) < 0.25 ?  more : 1;
            for(int i = 0; i < n_rep; i++)
            {
                if(CanAddAgent(world, at_pos))
                {
                    AddAgent(world, agent->type, at_pos, agent);
                }
            }
        }
    }
}

void
UpdateWorldChanges(World* world)
{
    // Process removed agents, sort indices descending and then actually remove.
    world->removed_agent_indices.Sort([](U32 a, U32 b) -> int {return b-a;});
    for(int remove_idx : world->removed_agent_indices)
    {
        Agent* agent = world->agents[remove_idx];
        world->brain_pool->Free(agent->brain);
        world->agent_pool->Free(agent);
        world->agents.RemoveIndexUnordered(remove_idx);
    }
    world->removed_agent_indices.Clear();

    world->ticks++;
    world->lifespan_arena_swap_ticks--;
    if(world->lifespan_arena_swap_ticks <= 0)
    {
        SwapLifespanArenas(world);
        world->lifespan_arena_swap_ticks = world->max_lifespan;
    }
}

void 
UpdateWorld(World* world)
{
    for(int agent_idx = 0; agent_idx < world->agents.size; agent_idx++)
    {
        Agent* agent = world->agents[agent_idx];

        RayCollision collision;
        agent->brain->input.Set(0);
        // Raycasting and updating eye sensors.
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
            if(eye->hit_type != 0)
            {
                agent->brain->input[eye_idx*3+eye->hit_type-1] = 1.0f;
                agent->brain->input[eye_idx*3+2] = 1.2f*(1.0f - eye->distance/50.0f);
            }
        }
        agent->brain->input[agent->brain->input.n-1] = 1.0f;

        UpdateBrain(agent);
        UpdateMovement(world, agent);

        agent->energy--;
        agent->ticks_alive++;
        if(agent->energy <= 0 || agent->ticks_alive >= world->max_lifespan)
        {
            RemoveAgent(world, agent_idx);
        }
        agent->ticks_until_reproduce--;
        if(agent->ticks_until_reproduce <= 0)
        {
            agent->ticks_until_reproduce = GetTicksUntilReproduction(world, agent->type);
            Vec2 at_pos = agent->pos+V2(0.5f, 0.5f);
            int more = agent->type==AgentType_Carnivore ? 4 : 2;
            int n_rep = RandomR32Debug(0, 1) < 0.25 ?  more : 1;
            for(int i = 0; i < n_rep; i++)
            {
                if(CanAddAgent(world, at_pos))
                {
                    AddAgent(world, agent->type, at_pos, agent);
                }
            }
        }
    }

    // Process removed agents, sort indices descending and then actually remove.
    world->removed_agent_indices.Sort([](U32 a, U32 b) -> int {return b-a;});
    for(int remove_idx : world->removed_agent_indices)
    {
        Agent* agent = world->agents[remove_idx];
        world->brain_pool->Free(agent->brain);
        world->agent_pool->Free(agent);
        world->agents.RemoveIndexUnordered(remove_idx);
    }
    world->removed_agent_indices.Clear();

    world->ticks++;
    world->lifespan_arena_swap_ticks--;
    if(world->lifespan_arena_swap_ticks <= 0)
    {
        SwapLifespanArenas(world);
        world->lifespan_arena_swap_ticks = world->max_lifespan;
    }
}

bool
HerbivoreCarnivoreCollision(Agent* herbivore, Agent* carnivore)
{
    Assert(herbivore->type==AgentType_Herbivore);
    Assert(carnivore->type==AgentType_Carnivore);

    if(IsRefractory(herbivore))
    {
        return false;
    }

    if(IsCharging(carnivore))
    {
        // Actual hit!
        herbivore->energy-=global_settings.energy_transfer_on_hit;
        carnivore->energy+=global_settings.energy_transfer_on_hit;
        return false;
    }

    return true;
}

static inline void
CheckAgentCollisions(Agent* agent0, Agent* agent1)
{
    Vec2 diff = agent1->pos - agent0->pos;
    R32 l2 = V2Len2(diff);
    R32 radsum = agent0->radius+agent1->radius;
    if(l2 < radsum*radsum)
    {
        bool resolve = true;
        if(agent0->type==AgentType_Carnivore && agent1->type==AgentType_Herbivore)
        {
            resolve = HerbivoreCarnivoreCollision(agent1, agent0);
        }
        else if(agent0->type==AgentType_Herbivore && agent1->type==AgentType_Carnivore)
        {
            resolve = HerbivoreCarnivoreCollision(agent0, agent1);
        }
        // Resolve collision
        if(resolve)
        {
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
    U32 eye_color = RGBAColor(255, 255, 255, 255);

    U32 pupil_color = RGBAColor(0,0,0,255);
    R32 pupil_r = eye_r*0.7f;

    // Eyes
    PushNGon(mesh, agent->pos+dir*eye_d+perp*eye_d, eye_r, eye_sides, eye_orientation, uv, eye_color);
    PushNGon(mesh, agent->pos+dir*eye_d-perp*eye_d, eye_r, eye_sides, eye_orientation, uv, eye_color);

    // Pupils
    PushNGon(mesh, agent->pos+dir*eye_d+perp*eye_d, pupil_r, eye_sides, eye_orientation, uv, pupil_color);
    PushNGon(mesh, agent->pos+dir*eye_d-perp*eye_d, pupil_r, eye_sides, eye_orientation, uv, pupil_color);
}

void 
RenderWorld(World* world, Mesh2D* mesh, Camera2D* cam, ColorOverlay color_overlay)
{
    // Draw world as square
    R32 world_rect_width = 2.0f/cam->scale;
    PushLineRect(mesh, V2(0,0), world->size, world_rect_width, V2(0,0), V2(0,0), RGBAColor(150, 150, 150, 150));

    // Which agents are visible. Omg this is stupid should be done using chunks.
    world->visible_agent_indices.Clear();
    for(int agent_idx = 0; agent_idx < world->agents.size; agent_idx++)
    {
        Agent* agent = world->agents[agent_idx];
        if(InBounds({cam->pos-cam->size/2.0f, cam->size}, agent->pos))
        {
            world->visible_agent_indices.PushBack(agent_idx);
        }
    }

    for(int i = 0; i < world->visible_agent_indices.size; i++)
    {
        Agent* agent = world->agents[world->visible_agent_indices[i]];

        R32 r = agent->radius;
        Vec2 uv = V2(0,0);
        Vec2 dir = V2Polar(agent->orientation, 1.0f);
        Vec2 perp = V2(dir.y, -dir.x);
        Vec4 herbivore_color = V4(0, 1, 0, 1);
        Vec4 carnivore_color = V4(1, 0, 0, 1);
        Vec4 color;

        // Overwrite for gene info
        switch(color_overlay)
        {
            case ColorOverlay_AgentGenes:
            {
                color = ColorToVec4(agent->gene_color);
            } break;

            default:
            {
                color = agent->type==AgentType_Herbivore ? herbivore_color : carnivore_color;
            } break;
        }

        if(IsCharging(agent))
        {
            // If charging we know its a carnivore now. Set bright red.
            color = V4(1.0f, 0.3f, 0.3f, 1.0f);
            color.a = 1.0f;
        }
        else if (IsRefractory(agent))
        {
            color = 0.5f * color;
            color.a = 1.0f;
        }

        // TODO: Better just make two completely separate loops
        if(cam->scale > 3)
        {
            if(agent->type==AgentType_Carnivore)
            {
                I32 sides = 3;
                R32 vel_len = V2Len(agent->vel); 
                R32 r = agent->radius/vel_len;
                Vec2 axis0 = agent->vel*r;
                Vec2 axis1 = V2(-axis0.y, axis0.x);
                axis0=(axis0*(1.0+vel_len));
                PushNGon(mesh, agent->pos, sides, axis0, axis1, uv, Vec4ToColor(color));
            }
            else
            {
                I32 sides = 5;
                R32 vel_len = V2Len(agent->vel); 
                R32 r = agent->radius/vel_len;
                Vec2 axis0 = agent->vel*r;
                Vec2 axis1 = V2(-axis0.y, axis0.x);
                axis0=(axis0*(1.0+vel_len));
                PushNGon(mesh, agent->pos, sides, axis0, axis1, uv, Vec4ToColor(color));
            }
            RenderDetails(mesh, agent);
        }
        else
        {
            I32 sides = 3;
            PushNGon(mesh, agent->pos, agent->radius, sides, agent->orientation, uv, Vec4ToColor(color));
        }
    }
}

void 
DrawChunks(World* world, Mesh2D* mesh, Camera2D* cam)
{
    Vec2 chunk_dims = V2(world->chunk_size, world->chunk_size);
    for(int chunk_idx = 0; chunk_idx < world->chunks.size; chunk_idx++)
    {
        Chunk* chunk = &world->chunks[chunk_idx];
        U32 color = chunk->agent_indices.size > 0 ? RGBAColor(0, 255, 0, 255) : RGBAColor(80, 80, 80, 80);
        PushLineRect(mesh, chunk->pos, chunk_dims, 1.0f/cam->scale, V2(0,0), V2(0,0), color);
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
SortAgentsIntoMultipleChunks(World* world)
{
    ClearChunks(world);

    for(int agent_idx = 0; agent_idx < world->agents.size; agent_idx++)
    {
        Agent* agent = world->agents[agent_idx];
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

Agent* 
AddAgent(World* world, AgentType type, Vec2 pos, Agent* parent)
{
    Agent* agent = world->agent_pool->Alloc();
    *agent = Agent{};
    world->agents.PushBack(agent);
    int agent_idx = world->agents.size-1;
    agent->pos = pos;
    agent->type = type;
    agent->radius = 1.2f;
    agent->id = 1;          // TODO: Use entity ids
    agent->is_alive = true;
    
    world->num_agenttype[type]++;

    MemoryArena* brain_arena = world->lifespan_arena;

    // Eyes
    int n_eyes = 4;
    R32 agent_fov = 0.6f;
    agent->eyes = CreateArray<AgentEye>(brain_arena, n_eyes);
    for(int i = 0; i < n_eyes; i++)
    {
        AgentEye* eye = agent->eyes.PushBack();
        eye->orientation = -agent_fov/2.0f + i*agent_fov/(n_eyes-1);
    }

    // Brain
    Brain* brain = world->brain_pool->Alloc();
    int inputs = n_eyes*3+1;
    int outputs = 4;
    R32 mutation_rate = world->mutation_rate;
    brain->gene = VecR32Create(brain_arena, inputs*outputs);
    if(parent)
    {
        brain->gene.CopyFrom(parent->brain->gene);
    }
    else
    {
        brain->gene.Set(0);
        brain->gene.AddNormal(0, 0.5f);
    }
    brain->gene.AddNormal(0, mutation_rate);
    brain->input = VecR32Create(brain_arena, inputs);
    brain->output = VecR32Create(brain_arena, outputs);
    I64 offset = 0;
    brain->weights = brain->gene.ShapeAs(inputs, outputs, offset);
    agent->brain = brain;

    // Here the gene is known, calculate a nice color to visualize the gene.
    R32 color_calc[3] = {0,0,0};
    for(int i = 0; i < brain->gene.n; i++)
    {
        color_calc[i%3] += Abs(brain->gene[i]);
    }

    // Stupid way to calculate max of 3 numbers. Should make library function
    // for this.
    R32 max_color = Max(color_calc[0], Max(color_calc[1], color_calc[2]));
    for(int i = 0; i < 3; i++)
    {
        color_calc[i] /= max_color;
    }

    R32 hue = fmodf(color_calc[0]*480.0f, 360.0f);
    agent->gene_color = HSVAToRGBA(hue, color_calc[1], color_calc[2], 1.0f);

    agent->ticks_until_reproduce = GetTicksUntilReproduction(world, agent->type) + (int)RandomR32Debug(-80, 40);
    agent->energy = GetInitialEnergy(world, agent->type);

    return agent;
}

void
RemoveAgent(World* world, U32 agent_idx)
{
    Agent* agent = world->agents[agent_idx];
    agent->is_alive = false;
    world->num_agenttype[agent->type]--;

    world->removed_agent_indices.PushBack(agent_idx);
}

Agent* 
SelectFromWorld(World* world, Vec2 pos, R32 extra_radius)
{
    Chunk* chunk = GetChunkAt(world, pos);
    for(U32 agent_idx : chunk->agent_indices)
    {
        Agent* agent = world->agents[agent_idx];
        R32 r2 = V2Dist2(agent->pos, pos);
        R32 target_rad = agent->radius+extra_radius;
        if(r2 < target_rad*target_rad)
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
        Agent* a0 = world->agents[a0_idx];

        for(int j = 0; j < chunk1->agent_indices.size; j++)
        {
            U32 a1_idx = chunk1->agent_indices[j];
            Agent* a1 = world->agents[a1_idx];
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
        Agent* a0 = world->agents[a0_idx];

        for(int j = i+1; j < center_chunk->agent_indices.size; j++)
        {
            U32 a1_idx = center_chunk->agent_indices[j];
            Agent* a1 = world->agents[a1_idx];
            CheckAgentCollisions(a0, a1);
        }
    }
}

World* 
CreateWorld(MemoryArena* arena)
{
    SimulationSettings* settings = &global_settings;
    World* world = PushStruct(arena, World);
    *world = World{};
    world->arena = arena;
    world->chunk_size = settings->chunk_size;
    world->x_chunks = settings->x_chunks;
    world->y_chunks = settings->y_chunks;
    world->size = world->chunk_size*V2(world->x_chunks, world->y_chunks);

    world->lifespan_arena = CreateSubArena(arena, MegaBytes(32));
    world->lifespan_arena_old = CreateSubArena(arena, MegaBytes(32));

    int max_agents = settings->max_agents;
    int n_initial_agents = settings->n_initial_agents;

    // TODO: This is a heuristic. Do something better.
    int max_agents_in_chunk = (int)(world->chunk_size*world->chunk_size*2);

    world->max_lifespan = 60*60;
    world->lifespan_arena_swap_ticks = world->max_lifespan;
    world->herbivore_reproduction_ticks = 60*10;
    world->herbivore_initial_energy = 60*11;

    world->carnivore_reproduction_ticks = 60*26;
    world->carnivore_initial_energy = 60*25;

    world->mutation_rate = settings->default_mutation_rate;

    world->max_eyes_per_agent = 6;

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

    world->agents = CreateArray<Agent*>(world->arena, max_agents);
    world->visible_agent_indices = CreateArray<U32>(world->arena, max_agents);
    world->removed_agent_indices = CreateArray<U32>(world->arena, max_agents);

    // Stupid way to calculate number of blocks. This whole part is stupid, I
    // just need a simple memory pool. Not this bitted thing I dont know why I
    // made it.
    U32 n_blocks = max_agents/32+1;
    world->agent_pool = CreateBittedMemoryPool<Agent>(arena, n_blocks);
    world->brain_pool = CreateBittedMemoryPool<Brain>(arena, n_blocks);

    for(int i = 0; i < n_initial_agents; i++)
    {
        AgentType type = i < n_initial_agents/2 ? AgentType_Carnivore : AgentType_Herbivore;
        Agent* agent = AddAgent(world, type, RandomVec2Debug(V2(0,0), world->size));
        agent->orientation = RandomR32Debug(-M_PI, M_PI);
    }
    return world;
}
