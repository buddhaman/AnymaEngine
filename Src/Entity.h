#pragma once

#include "AnymUtil.h"

enum class EntityType
{
    Agent,
    Plant,
};

struct Entity
{
    Vec2 pos;
    R32 radius = 0.0f;
    EntityType type;
    bool removed;
};
