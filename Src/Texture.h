#pragma once

#include "TimVectorMath.h"
#include "AnymUtil.h"
#include "Array.h"

// Forward declarations
struct TextureAtlas;

struct AtlasRegion
{
    TextureAtlas* atlas;
    char *name;
    int width;
    int height;
    int x;
    int y;
    Vec2 pos;
    Vec2 size;
};

struct TextureAtlas
{
    // TODO: pull this out and create image class.
    int width;
    int height;
    U32 *image;

    U32 handle;
    Array<AtlasRegion> regions;
};

TextureAtlas *
MakeDefaultTexture(MemoryArena *arena, int circleRadius);