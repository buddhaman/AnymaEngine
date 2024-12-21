#pragma once

#include "AnymUtil.h"
#include "Array.h"
#include "TMath.h"
#include "Linalg.h"
#include "Skeleton.h"
#include "Genome.h"
#include "Brain.h"

// Forward declarations
struct World;

using EntityID = U32;

enum AgentType
{
    AgentType_None = 0,
    AgentType_Herbivore = 1,
    AgentType_Carnivore = 2,
    AgentType_Num
};

struct AgentEye
{
    Ray ray;
    R32 distance;
    AgentType hit_type;
    R32 orientation;
};

struct PhenoType
{
    int n_backbones = 0;
    VecR32 backbone_radius;

    Array<U32> has_leg;
    VecR32 knee_size;
    VecR32 foot_size;
    VecR32 step_radius;

    // Later we want to customize number of limbs.
    R32 elbow_size = 0;
    R32 hand_size = 0;
    U32 color;
};

PhenoType*
CreatePhenotype(MemoryArena* arena, int max_backbones);

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

struct Agent
{
    Vec2 pos; 
    Vec2 vel;

    R32 orientation;
    R32 radius;
    I32 charge;
    I32 charge_refractory;

    Head head;
    MainBody body;
    Array<Leg> legs;
    Skeleton* skeleton;

    Array<AgentEye> eyes;
    Brain* brain;
    PhenoType* phenotype;

    AgentType type;
    EntityID id;

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

    bool is_alive;
};

static inline U32
GetAgentColor(AgentType type)
{
    switch(type)
    {
    case AgentType_Carnivore: return Color_Red;
    case AgentType_Herbivore: return Color_Green;
    default: return Color_White;
    }
}


void
UpdateMovement(World* world, Agent* agent);

I32
GetTicksUntilReproduction(World* world, AgentType type);

I32
GetInitialEnergy(World* world, AgentType type);

static inline bool
IsCharging(Agent* agent) { return agent->charge; }

static inline bool
IsRefractory(Agent* agent) { return agent->charge_refractory != 0; }

