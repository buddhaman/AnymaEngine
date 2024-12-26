#pragma once

#include "AnymUtil.h"

enum EntityType
{
    Entity_Agent,
    Entity_Plant,
};

struct Entity
{
    Vec2 pos;
    R32 radius;
    EntityType type;
};
