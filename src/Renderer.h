#pragma once

#include "AnymUtil.h"
#include "Mesh2D.h"
#include "Texture.h"
#include "Shader.h"
#include "Camera2D.h"

// Renderer encapsulates a bunch of other rendering utilities to make drawing simple shapes more easy.
struct Renderer
{
    TextureAtlas* atlas;
    AtlasRegion* circle;
    AtlasRegion* square;
    Mesh2D mesh;
    Shader shader;
};

void
RenderLine(Renderer* renderer, Vec2 from, Vec2 to, R32 line_width, U32 color);

void
RenderCircle(Renderer* renderer, Vec2 center, R32 radius, U32 color);

void
Render(Renderer* renderer, Camera2D* cam, int width, int height);

Renderer* 
CreateRenderer(MemoryArena* arena);