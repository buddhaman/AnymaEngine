#include "World.h"

EntityID
CastRay(World* world, Ray ray, EntityID exclude_id)
{
    R32 minDist = 10000.0f;
    for(int i = 0; i < world->agents.size; i++)
    {
        Agent* agent = &world->agents[i];
        if(agent->id==exclude_id) continue;
        
        Vec2 intersection;
        RayCircleIntersect(ray, {agent->pos, agent->radius}, &intersection);
        
    }
    return 0;
}

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

    }

#if 1
    // Collision detection
    for(int i = 0; i < world->agents.size-1; i++)
    {
        Agent* agent0 = &world->agents[i];
        for(int j = i+1; j < world->agents.size; j++)
        {
            Agent* agent1 = &world->agents[j];
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
    }
#endif
}

static inline void
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
        PushLine(mesh, eye->ray.pos, eye->ray.pos + eye->distance*eye->ray.direction, ray_width, uv, uv, eye->color);
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
    for(int i = 0; i < world->agents.size; i++)
    {
        Agent* agent = &world->agents[i];

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
        PushLineRect(mesh, chunk->pos, chunk_dims, 1.0f/cam->scale, V2(0,0), V2(0,0), 0xffffffff);
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
        int x_chunk = floor(agent->pos.x/world->chunk_size);
        int y_chunk = floor(agent->pos.y/world->chunk_size);
        GetChunk(world, x_chunk, y_chunk)->agent_indices.PushBack(agent_idx);
    }
}

void 
InitWorld(World* world)
{
    world->arena = CreateMemoryArena(MegaBytes(256));
    world->chunk_size = 100;
    world->x_chunks = 4;
    world->y_chunks = 4;
    world->size = world->chunk_size*V2(world->x_chunks, world->y_chunks);

    int n_agents = 400;

    world->chunks = CreateArray<Chunk>(world->arena, world->x_chunks*world->y_chunks);
    for(int y = 0; y < world->y_chunks; y++)
    for(int x = 0; x < world->x_chunks; x++)
    {
        Chunk* chunk = world->chunks.PushBack();
        chunk->pos = V2(x*world->chunk_size, y*world->chunk_size);
        chunk->x_idx = x;
        chunk->y_idx = y;
        chunk->agent_indices = CreateArray<U32>(world->arena, n_agents);
    }

    world->agents = CreateArray<Agent>(world->arena, 64000);
    for(int i = 0; i < n_agents; i++)
    {
        Agent* agent = world->agents.PushBack();
        *agent = {0};
        agent->pos.x = GetRandomR32Debug(0, world->size.x);
        agent->pos.y = GetRandomR32Debug(0, world->size.y);
        agent->type = i < n_agents/2 ? AgentType_Carnivore : AgentType_Herbivore;
        agent->radius = 1.2f;
        agent->id = i+1;

        int n_eyes = 4;
        R32 agent_fov = 0.8f;
        agent->eyes = CreateArray<AgentEye>(world->arena, n_eyes);
        for(int i = 0; i < n_eyes; i++)
        {
            AgentEye* eye = agent->eyes.PushBack();
            eye->orientation = -agent_fov/2.0f + i*agent_fov/(n_eyes-1);
        }
    }
}
