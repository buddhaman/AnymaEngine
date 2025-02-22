#include "World.h"

#include "Noise.h"
#include "Skeleton.h"

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

// Now this algorithm can double check agents belonging to multiple cells. Fix this.
// Idea: first collect all agents, sort with pairs where the first index is the length 
Agent* 
CastRay(World* world, Ray ray, R32 ray_length, RayCollision* collision, Agent* exclude_agent)
{
    Vec2 at = ray.pos;
    int x_cell = GetXCell(world, at.x);
    int y_cell = GetYCell(world, at.y);
    if( x_cell < 0 || 
        y_cell < 0 || 
        x_cell >= world->x_cells || 
        y_cell >= world->y_cells) 
    {
        return nullptr;
    }

    R32 traversed = 0.0f;
    int x_dir = ray.dir.x < 0.0f ? -1 : 1;
    int y_dir = ray.dir.y < 0.0f ? -1 : 1;
    R32 half_cell = world->cell_size/2.0f;

    while(traversed < ray_length)
    {
        Cell* cell = GetCell(world, x_cell, y_cell);
        
        R32 min_intersect_dist = NumLimits<R32>::Max();
        Agent* hit = nullptr;
        for(U32 agent_idx : cell->agent_indices)
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

        // Move to next cell.
        R32 x_edge = (cell->x_idx + 0.5f)*world->cell_size + half_cell*x_dir;
        R32 y_edge = (cell->y_idx + 0.5f)*world->cell_size + half_cell*y_dir;
        R32 x_trav = (x_edge - at.x)/ray.dir.x;
        R32 y_trav = (y_edge - at.y)/ray.dir.y;
        if(x_trav < y_trav)
        {
            traversed += x_trav;
            at = ray.pos + traversed*ray.dir;
            x_cell += x_dir;
            if(x_cell < 0 || x_cell >= world->x_cells) break;
        }
        else
        {
            traversed += y_trav;
            at = ray.pos + traversed*ray.dir;
            y_cell += y_dir;
            if(y_cell < 0 || y_cell >= world->y_cells) break;
        }
    }

    return nullptr;
}

bool 
CanAddAgent(World* world, Vec2 pos)
{
    Cell* cell = GetCellAt(world, pos);
    if(world->agents.IsFull()) return false;
    if(cell->agent_indices.IsFull()) return false;
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
        agent->brain->input.Set(0);
#if 0
        RayCollision collision;

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
#else      

        // Instead of casting a ray find nearest agents and give the relative angle (in fov) and 
        // TODO: Get only agents in the cells that intersect the fov circle
        // section. Find nearest herbivore and nearest carnivore.
        agent->nearest_carnivore = nullptr;
        agent->nearest_herbivore = nullptr;
        R32 sight_radius = agent->sight_radius;
        int min_cell_x = Max(0, GetXCell(world, agent->pos.x - sight_radius));
        int min_cell_y = Max(0, GetYCell(world, agent->pos.y - sight_radius));
        int max_cell_x = Min(world->x_cells-1, GetXCell(world, agent->pos.x + sight_radius));
        int max_cell_y = Min(world->y_cells-1, GetYCell(world, agent->pos.y + sight_radius));
        // We dont take square cause later we want to take agent radius into account.
        agent->nearest_carnivore_distance = sight_radius;
        agent->nearest_herbivore_distance = sight_radius;
        for(int y_cell = min_cell_y; y_cell <= max_cell_y; y_cell++)
        for(int x_cell = min_cell_x; x_cell <= max_cell_x; x_cell++)
        {
            // Iterate over all agents and check if they fall within the fov and radius.
            Cell* cell = GetCell(world, x_cell, y_cell);
            for(U32 agent_idx : cell->agent_indices)
            {
                Agent* other = world->agents[agent_idx];
                if(other==agent) continue;
                Vec2 diff = other->pos - agent->pos;
                R32 angle_diff = atan2f(diff.y, diff.x);
                R32 real_diff = NormalizeAngle(angle_diff - agent->orientation);
                if(-agent->fov < real_diff && real_diff < agent->fov)
                {
                    R32 distance2 = V2Len2(diff);

                    // Set as nearest carnivore or herbivore if possible.
                    if(other->type==AgentType_Carnivore)
                    {
                        R32 l = agent->nearest_carnivore_distance;
                        if(distance2 < l*l)
                        {
                            // Visible
                            agent->nearest_carnivore_distance = sqrtf(distance2);
                            agent->nearest_carnivore_angle = real_diff;
                            agent->nearest_carnivore = other;
                        }
                    }
                    else
                    {
                        R32 l = agent->nearest_herbivore_distance;
                        if(distance2 < l*l)
                        {
                            // Visible
                            agent->nearest_herbivore_distance = sqrtf(distance2);
                            agent->nearest_herbivore_angle = real_diff;
                            agent->nearest_herbivore = other;
                        }
                    }
                }
            }
        }

        // Set brain parameters based on nearest herbivore and carnivore.
        R32 carn_distance_activation = 1.0f - 2.0f*agent->nearest_carnivore_distance/sight_radius;
        R32 carn_angle_activation = agent->nearest_carnivore_angle / agent->fov;
        R32 herb_distance_activation = 1.0f - 2.0f*agent->nearest_herbivore_distance/sight_radius;
        R32 herb_angle_activation = agent->nearest_herbivore_angle / agent->fov;

        // Calculate herbivore relative x and y velocity. For herbivores to flock and for carnivores to track.
        Vec2 herb_vel = V2(0,0);
        if(agent->nearest_herbivore)
        {
            Vec2 rely_axis = V2Polar(agent->orientation, 1.0);
            Vec2 relx_axis = V2(-rely_axis.y, rely_axis.x);

            // Transform other agents velocity into this agents' local coordinate system.
            // TODO: Clamp this velocity so it does not become too big.
            Agent* other = agent->nearest_herbivore;
            herb_vel = V2(V2Dot(relx_axis, other->vel), V2Dot(rely_axis, other->vel));
        }

        Brain* brain = agent->brain;
        brain->input[0] = carn_distance_activation;
        brain->input[1] = carn_angle_activation;
        brain->input[2] = herb_distance_activation;
        brain->input[3] = herb_angle_activation;

        // relative movement of nearest herbivore
        brain->input[4] = herb_vel.x;
        brain->input[5] = herb_vel.y;
        //brain->input[4] = sinf((R32)(world->ticks / 60.0f));
#endif

        UpdateBrain(agent->brain);
    }
}

void
UpdateAgentBehavior(Camera2D* cam, World* world, I32 from_idx, I32 to_idx)
{
    for(I32 agent_idx = from_idx; agent_idx < to_idx; agent_idx++)
    {
        Agent* agent = world->agents[agent_idx];
        UpdateMovement(world, agent);

        agent->energy--;
        agent->ticks_alive++;
        if(agent->energy <= 0 || agent->ticks_alive >= world->max_lifespan)
        {
            if(world->spawn_particles 
                && cam->scale > 3
                && InBounds(cam->bounds, agent->pos))
            {
                // Try spawn particle
                R32 vel = 1.4f;
                U32 color = GetAgentColor(agent->type);
                for(int i = 0; i < 5; i++)
                {
                    TrySpawnParticle(world, agent->pos, RandomVec2Debug(V2(-vel, -vel), V2(vel, vel)), 0.45f, color);
                }
            }
            RemoveAgent(world, agent_idx);
        }
        agent->ticks_until_reproduce--;
        if(agent->ticks_until_reproduce <= 0)
        {
            agent->ticks_until_reproduce = GetTicksUntilReproduction(world, agent->type);
            if(world->num_agenttype[agent->type] < global_settings.max_agents*0.9f)
            {
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
}

void
UpdateWorldChanges(World* world)
{
    // Process removed agents, sort indices descending and then actually remove.
    world->removed_agent_indices.Sort([](U32 a, U32 b) -> int {return b-a;});
    for(int remove_idx : world->removed_agent_indices)
    {
        Agent* agent = world->agents[remove_idx];
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

        // Set herbivore to refractory
        herbivore->charge_refractory = global_settings.charge_refractory_period;

        // Add a nice extra impulse to the herbivore
        herbivore->vel += 10*carnivore->vel;

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
RenderEyeRays(Mesh2D* mesh, Agent* agent, Vec2 uv)
{
    for(int i = 0; i < agent->eyes.size; i++)
    {
        AgentEye* eye = &agent->eyes[i];
        R32 ray_width = 0.1f;
        U32 color = GetAgentColor(eye->hit_type);
        PushLine(mesh, eye->ray.pos, eye->ray.pos + eye->distance*eye->ray.dir, ray_width, uv, V2(0,0), color);
    }

    // Draw fov cone
    PushCircularSector(mesh, agent->pos, agent->sight_radius, 24, agent->orientation-agent->fov, agent->orientation+agent->fov, uv, RGBAColor(255, 255, 255, 50));

    if(agent->nearest_carnivore)
    {
        PushLine(mesh, agent->pos, agent->nearest_carnivore->pos, 0.1f, uv, V2(0,0), RGBAColor(255, 0, 0, 255));
    }
    if(agent->nearest_herbivore)
    {
        PushLine(mesh, agent->pos, agent->nearest_herbivore->pos, 0.1f, uv, V2(0,0), RGBAColor(0, 255, 0, 255));
    }
}

void
RenderDetails(Mesh2D* mesh, Agent* agent, Vec2 uv)
{
    R32 r = agent->radius;
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
RenderHealth(Mesh2D* mesh, World* world, Agent* agent, Vec2 uv)
{
    R32 r = agent->radius;
    R32 full_bar_width = r*2;
    R32 bar_height = r/2.0f;
    // Divide by herbivore initial energy since this is more than carnivore initial energy.
    R32 health_width = full_bar_width*((R32)agent->energy) / ((R32)global_settings.carnivore_initial_energy);
    Vec2 u = V2(0,0);
    PushRect(mesh, agent->pos-V2(full_bar_width/2.0f, r*1.5f), V2(health_width, bar_height), u, V2(0,0), RGBAColor(0, 255, 0, 255));
}

void 
RenderWorld(World* world, Mesh2D* mesh, Camera2D* cam, Vec2 uv, ColorOverlay color_overlay)
{
    // Draw world as square
    R32 world_rect_width = 2.0f/cam->scale;
    PushLineRect(mesh, V2(0,0), world->size, world_rect_width, uv, V2(0,0), RGBAColor(150, 150, 150, 150));

    // Which agents are visible. Omg this is stupid should be done using cells.
    world->visible_agent_indices.Clear();
    for(int agent_idx = 0; agent_idx < world->agents.size; agent_idx++)
    {
        Agent* agent = world->agents[agent_idx];
        if(InBounds(cam->bounds, agent->pos))
        {
            world->visible_agent_indices.PushBack(agent_idx);
        }
    }

    for(int i = 0; i < world->visible_agent_indices.size; i++)
    {
        Agent* agent = world->agents[world->visible_agent_indices[i]];

        R32 r = agent->radius;
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
                axis0=(axis0*(1.0f+vel_len));
                // PushNGon(mesh, agent->pos, sides, axis0, axis1, uv, Vec4ToColor(color));
                axis0 = V2Polar(agent->orientation, agent->radius);
                axis1 = V2(-axis0.y, axis0.x);
                PushNGon(mesh, agent->pos, sides, axis0, axis1, uv, Vec4ToColor(color));
            }
            else
            {
                I32 sides = 5;
                R32 vel_len = V2Len(agent->vel); 
                R32 r = agent->radius/vel_len;
                Vec2 axis0 = agent->vel*r;
                Vec2 axis1 = V2(-axis0.y, axis0.x);
                axis0=(axis0*(1.0f+vel_len));
                // PushNGon(mesh, agent->pos, sides, axis0, axis1, uv, Vec4ToColor(color));
                axis0 = V2Polar(agent->orientation, agent->radius);
                axis1 = V2(-axis0.y, axis0.x);
                PushNGon(mesh, agent->pos, sides, axis0, axis1, uv, Vec4ToColor(color));
            }
            RenderDetails(mesh, agent, uv);
            RenderHealth(mesh, world, agent, uv);

        }
        else
        {
            I32 sides = 3;
            PushNGon(mesh, agent->pos, agent->radius, sides, agent->orientation, uv, Vec4ToColor(color));
        }
    }

    // Render particles
    if(cam->scale > 3)
    {
        Vec2 uv = V2(0,0);
        for(int particle_idx = 0; particle_idx < world->particles.size; particle_idx++)
        {
            Particle& p = world->particles[particle_idx];
            I32 sides = 5;
            R32 vel_len = V2Len(p.vel); 
            R32 r = 1.2f*p.lifetime/vel_len;
            Vec2 axis0 = p.vel*r;
            Vec2 axis1 = V2(-axis0.y, axis0.x);
            axis0=(axis0*(1.0f+vel_len));
            PushNGon(mesh, p.pos, sides, axis0, axis1, uv, p.color);
        }
    }
}

void 
DrawCells(World* world, Mesh2D* mesh, Camera2D* cam, Vec2 uv)
{
    Vec2 cell_dims = V2(world->cell_size, world->cell_size);
    for(int cell_idx = 0; cell_idx < world->cells.size; cell_idx++)
    {
        Cell* cell = &world->cells[cell_idx];
        U32 color = cell->agent_indices.size > 0 ? RGBAColor(0, 255, 0, 255) : RGBAColor(80, 80, 80, 80);
        PushLineRect(mesh, cell->pos, cell_dims, 1.0f/cam->scale, uv, V2(0,0), color);
    }
}

static inline void
ClearCells(World* world)
{
    for(int cell_idx = 0; cell_idx < world->cells.size; cell_idx++)
    {
        world->cells[cell_idx].agent_indices.Clear();
    } 
}

void
SortAgentsIntoMultipleCells(World* world)
{
    ClearCells(world);

    for(int agent_idx = 0; agent_idx < world->agents.size; agent_idx++)
    {
        Agent* agent = world->agents[agent_idx];
        int min_x = Max(0, (int)((agent->pos.x-agent->radius)/world->cell_size));
        int max_x = Min(world->x_cells-1, (int)((agent->pos.x+agent->radius)/world->cell_size));
        int min_y = Max(0, (int)((agent->pos.y-agent->radius)/world->cell_size));
        int max_y = Min(world->y_cells-1, (int)((agent->pos.y+agent->radius)/world->cell_size));
        for(int y = min_y; y <= max_y; y++)
        for(int x = min_x; x <= max_x; x++)
        {
            Cell* cell = GetCell(world, x, y);
            cell->agent_indices.PushBack(agent_idx);
        }
    }
}

Agent* 
AddAgent(World* world, AgentType type, Vec2 pos, Agent* parent)
{
    Agent* agent = PushNewStruct(world->lifespan_arena, Agent);
    world->agents.PushBack(agent);
    I64 agent_idx = world->agents.size-1;
    agent->pos = pos;
    agent->type = type;
    agent->is_alive = true;
    agent->fov = type == AgentType_Carnivore ? 0.6f : 1.2f;
    agent->nearest_carnivore = nullptr;
    agent->nearest_herbivore = nullptr;
    agent->sight_radius = 40;
    
    world->num_agenttype[type]++;

    MemoryArena* arena = world->lifespan_arena;

    // Eyes
    int n_eyes = 4;
    agent->eyes = CreateArray<AgentEye>(arena, n_eyes);
    for(int i = 0; i < n_eyes; i++)
    {
        AgentEye* eye = agent->eyes.PushBack();
        eye->orientation = -agent->fov/2.0f + i*agent->fov/(n_eyes-1);
    }

    // Brain
    int inputs = n_eyes*3+1;
    int outputs = 4;
    R32 mutation_rate = global_settings.mutation_rate;
    Brain* brain = agent->brain = CreateBrain(arena, inputs, inputs*2+1, outputs, parent ? parent->brain : nullptr, mutation_rate);

    // Here the gene is known, calculate a nice color to visualize the gene.
    R32 color_calc[3] = {0,0,0};
    for(int i = 0; i < brain->gene.n; i++)
    {
        color_calc[i%3] += Abs(brain->gene[i]);
    }

    // Stupid way to calculate max of 3 numbers. Should make library function
    // for this.
    R32 max_color = Max(color_calc[0], color_calc[1], color_calc[2]);
    for(int i = 0; i < 3; i++)
    {
        color_calc[i] /= max_color;
    }

    R32 hue = fmodf(color_calc[0]*480.0f, 360.0f);
    agent->gene_color = HSVAToRGBA(hue, color_calc[1], color_calc[2], 1.0f);

    agent->ticks_until_reproduce = GetTicksUntilReproduction(world, agent->type) + (int)RandomR32Debug(-80, 40);
    agent->energy = GetInitialEnergy(world, agent->type);

    // Skeleton
    // TODO: Streamline skeleton creation. Tie it to the phenotype.
    agent->skeleton = CreateSkeleton(arena, 48, 96);
    if(parent==nullptr)
    {
        agent->phenotype = CreatePhenotype(arena, 4);
        InitRandomPhenotype(agent->phenotype);
    }
    else
    {
        agent->phenotype = CopyPhenotype(arena, parent->phenotype);
        MutatePhenotype(agent->phenotype);
    }
    InitAgentSkeleton(arena, agent);

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
    Cell* cell = GetCellAt(world, pos);
    for(U32 agent_idx : cell->agent_indices)
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
CellCollisions(World* world, Cell* cell0, Cell* cell1)
{
    for(int i = 0; i < cell0->agent_indices.size; i++)
    {
        U32 a0_idx = cell0->agent_indices[i];
        Agent* a0 = world->agents[a0_idx];

        for(int j = 0; j < cell1->agent_indices.size; j++)
        {
            U32 a1_idx = cell1->agent_indices[j];
            Agent* a1 = world->agents[a1_idx];
            CheckAgentCollisions(a0, a1);
        }
    }
}

void
CellCollisions(World* world, int center_cell_x, int center_cell_y)
{
    // First try, naive algorithm.
    Cell* center_cell = GetCell(world, center_cell_x, center_cell_y);

    bool has_right = center_cell_x < world->x_cells-1;
    bool has_down = center_cell_y < world->y_cells-1;
    if(has_right) 
    {
        CellCollisions(world, center_cell, GetCell(world, center_cell_x+1, center_cell_y));
    }
    if(has_down) 
    {
        CellCollisions(world, center_cell, GetCell(world, center_cell_x, center_cell_y+1));
    }
    if(has_down && has_right)
    {
        CellCollisions(world, center_cell, GetCell(world, center_cell_x+1, center_cell_y+1));
    }

    // Inside center cell.
    for(int i = 0; i < center_cell->agent_indices.size-1; i++)
    {
        U32 a0_idx = center_cell->agent_indices[i];
        Agent* a0 = world->agents[a0_idx];

        for(int j = i+1; j < center_cell->agent_indices.size; j++)
        {
            U32 a1_idx = center_cell->agent_indices[j];
            Agent* a1 = world->agents[a1_idx];
            CheckAgentCollisions(a0, a1);
        }
    }
}

Particle *
TrySpawnParticle(World* world, Vec2 pos, Vec2 vel, R32 lifetime, U32 color)
{
    if(world->particles.IsFull())
    {
        return nullptr;
    }

    Particle* p = world->particles.PushBack();
    p->pos = pos;
    p->vel = vel;
    p->lifetime = lifetime;
    p->color = color;
    return p;
}

void
UpdateParticles(World* world)
{
    // Clear all particles if no particles should be spawned. Particles are only
    // cosmetic.
    if(!world->spawn_particles)
    {
        world->particles.Clear();
        return;
    }

    // Update particles 
    for(int particle_idx = 0; particle_idx < world->particles.size; particle_idx++)
    {
        Particle& p = world->particles[particle_idx];
        p.lifetime -= 1.0f/60.0f;
        p.pos += p.vel;
    }

    // Remove particles at end of lifetime.
    for(int particle_idx = (int)world->particles.size-1; particle_idx >= 0; particle_idx--)
    {
        Particle& p = world->particles[particle_idx];
        if(p.lifetime <= 0.0f)
        {
            world->particles.RemoveIndexUnordered(particle_idx);
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
    world->cell_size = settings->cell_size;
    world->x_cells = settings->x_cells;
    world->y_cells = settings->y_cells;
    world->size = world->cell_size*V2((R32)world->x_cells, (R32)world->y_cells);

    U64 lifespan_arena_size = MegaBytes(384);
    world->lifespan_arena = CreateSubArena(arena, lifespan_arena_size);
    world->lifespan_arena_old = CreateSubArena(arena, lifespan_arena_size);

    int max_agents = settings->max_agents;
    int n_initial_agents = settings->n_initial_agents;
    // TODO: This is a heuristic. Do something better.
    int max_agents_in_cell = (int)(world->cell_size*world->cell_size*2);

    world->max_lifespan = 60*60;
    world->lifespan_arena_swap_ticks = world->max_lifespan;

    world->max_eyes_per_agent = 6;

    PerlinNoise noise;
    noise.seed = RandomIntDebug(0, 10000);
    world->cells = CreateArray<Cell>(world->arena, world->x_cells*world->y_cells);
    for(int y = 0; y < world->y_cells; y++)
    for(int x = 0; x < world->x_cells; x++)
    {
        Cell* cell = world->cells.PushBack();
        cell->pos = V2(x*world->cell_size, y*world->cell_size);
        cell->x_idx = x;
        cell->y_idx = y;
        R32 scale = 20.0f;
        R32 x_noise = cell->pos.x / world->size.x;
        R32 y_noise = cell->pos.y / world->size.y;
        R32 noise_value = Perlin2D(&noise, scale*x_noise, scale*y_noise, 0.2f, 4);
        cell->type = noise_value < 0.5f ? CellType_Sand : CellType_Grass;
        cell->agent_indices = CreateArray<U32>(world->arena, max_agents_in_cell);
    }

    // Iterate over corners
    world->cell_corner_colors = CreateArray<U32>(world->arena, (world->x_cells + 1) * (world->y_cells + 1));
    world->cell_corner_colors.Fill();
    for (int y = 0; y <= world->y_cells; y++) 
    {
        for (int x = 0; x <= world->x_cells; x++)
        {
            int kernel_radius = 2;

            int min_x = Max(0, x - kernel_radius);
            int min_y = Max(0, y - kernel_radius);
            int max_x = Min(world->x_cells - 1, x + kernel_radius - 1);
            int max_y = Min(world->y_cells - 1, y + kernel_radius - 1);

            int n_cells = Max(1, (max_x - min_x + 1) * (max_y - min_y + 1));

            Vec3 avg = V3(0, 0, 0);
            for (int yy = min_y; yy <= max_y; yy++)
            {
                for (int xx = min_x; xx <= max_x; xx++)
                {
                    Cell* cell = GetCell(world, xx, yy);
                    U32 type_color = GetCellTypeColor(cell->type);
                    Vec3 type_color_v3 = ColorToVec4(type_color).xyz;
                    avg += type_color_v3;
                }
            }

            avg /= ((R32)n_cells);

            // Set the interpolated color for the current corner
            world->cell_corner_colors[y * (world->x_cells + 1) + x] = Vec3ToColor(avg.r, avg.g, avg.b);
        }
    }

    world->agents = CreateArray<Agent*>(world->arena, max_agents);
    world->visible_agent_indices = CreateArray<U32>(world->arena, max_agents);
    world->removed_agent_indices = CreateArray<U32>(world->arena, max_agents);

    world->spawn_particles = true;
    world->particles = CreateArray<Particle>(1024);

    for(int i = 0; i < n_initial_agents; i++)
    {
        AgentType type = i < n_initial_agents/2 ? AgentType_Carnivore : AgentType_Herbivore;
        Agent* agent = AddAgent(world, type, RandomVec2Debug(V2(0,0), world->size));
        agent->orientation = RandomR32Debug(-(R32)M_PI, -(R32)M_PI);
    }
    return world;
}
