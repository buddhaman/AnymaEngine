#pragma once

#include "AnymUtil.h"
#include "Array.h"
#include "TMath.h"
#include "Linalg.h"
#include "Skeleton.h"
#include "Genome.h"
#include "Brain.h"
#include "Entity.h"

// Forward declarations
struct World;

enum class AgentType
{
    None = 0,
    Herbivore = 1,
    Carnivore = 2,
    Num
};

struct AgentEye
{
    Ray ray;
    R32 distance;
    int hit_type;
    R32 orientation;
};

struct PhenoType
{
    U32 size_in_bytes;

    int n_backbones = 0;
    VecR32 backbone_radius;

    Array<U32> has_leg;
    VecR32 knee_size;
    VecR32 foot_size;
    VecR32 step_radius;

    // Later we want to customize number of limbs.
    R32 head_size = 0;
    R32 elbow_size = 0;
    R32 hand_size = 0;
    U32 color;
};

PhenoType*
CreatePhenotype(MemoryArena* arena, int max_backbones);

void
InitRandomPhenotype(PhenoType* pheno);

PhenoType*
CopyPhenotype(MemoryArena* arena, PhenoType* source);

void
MutatePhenotype(PhenoType* pheno);

struct Leg
{
    // Idx of the end of the leg.
    int idx;    

    // Target offset in current agent local coordinates.
    Vec3 target_offset;

    // Current foot position in world coordinates, not local.
    Vec3 foot_pos;

    // Max distance this foot can have to the calculated offset.
    R32 r;
};

struct Head
{
    int idx;

    Vec3 target_position;
};

struct MainBody
{
    Array<int> body;
    Array<Vec3> target_positions;
};

struct Agent : public Entity
{
    Vec2 vel;

    R32 orientation;
    I32 charge;
    I32 charge_refractory;

    Head head;
    MainBody body;
    Array<Leg> legs;
    Leg l_arm;
    Leg r_arm;
    Skeleton* skeleton;

    Array<AgentEye> eyes;
    Brain* brain;
    PhenoType* phenotype;

    AgentType agent_type;

    I32 ticks_until_reproduce;
    I32 energy;
    I32 ticks_alive;

    // This is the color used to identify the gene. For visualization.
    U32 gene_color;
    R32 fov;
    R32 sight_radius;

    R32 nearest_herbivore_distance;
    R32 nearest_herbivore_angle;
    Agent* nearest_herbivore;

    R32 nearest_carnivore_distance;
    R32 nearest_carnivore_angle;
    Agent* nearest_carnivore;
};

static inline U32
GetAgentColor(AgentType type)
{
    switch(type)
    {
    case AgentType::Carnivore: return Color_Red;
    case AgentType::Herbivore: return Color_Green;
    default: return Color_White;
    }
}

void
UpdateMovement(World* world, Agent* agent);

void
RenderAgent(TiltedRenderer* renderer, Agent* agent);

I32
GetTicksUntilReproduction(World* world, AgentType type);

I32
GetInitialEnergy(World* world, AgentType type);

void 
InitAgentSkeleton(MemoryArena* arena, Agent* agent);

void
UpdateAgentSkeleton(Agent* agent);

static inline bool
IsCharging(Agent* agent) { return agent->charge; }

static inline bool
IsRefractory(Agent* agent) { return agent->charge_refractory != 0; }

Agent* 
CreateAgent(World* world, AgentType type, Vec2 pos, Agent* parent=nullptr);

void
RemoveAgent(World* world, U32 agent_idx);

