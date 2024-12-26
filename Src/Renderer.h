#pragma once

#include "AnymUtil.h"
#include "Mesh2D.h"
#include "Texture.h"
#include "Shader.h"
#include "Camera2D.h"
#include "TiltCamera.h"

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
RenderTrapezoid(Renderer* renderer, Vec2 from, R32 from_width, Vec2 to, R32 to_width, U32 color);

void
RenderCircle(Renderer* renderer, Vec2 center, R32 radius, U32 color);

void
RenderRect(Renderer* renderer, Vec2 center, Vec2 dims, AtlasRegion* region, U32 color);

void
Render(Renderer* renderer, Camera2D* cam, int width, int height);

Renderer* 
CreateRenderer(MemoryArena* arena);

// Renders in a tilted world.
struct TiltedRenderer
{
    Renderer* renderer;
    TiltedCamera cam;
};

TiltedRenderer* 
CreateTiltedRenderer(MemoryArena* arena);

void
RenderLine(TiltedRenderer* renderer, Vec3 from, Vec3 to, R32 line_width, U32 color);

void
RenderTrapezoid(TiltedRenderer* renderer, Vec3 from, R32 from_width, Vec3 to, R32 to_width, U32 color);

void
RenderCircle(TiltedRenderer* renderer, Vec3 center, R32 radius, U32 color);

void
RenderZRect(TiltedRenderer* renderer, Vec3 center, Vec2 dims, AtlasRegion* region, U32 color);

void
RenderZRect(TiltedRenderer* renderer, Vec3 center, Vec2 dims, AtlasRegion* region, 
            U32 color0, U32 color1, U32 color2, U32 color3);

void
RenderYRect(TiltedRenderer* renderer, Vec3 center, Vec2 dims, AtlasRegion* region, U32 color);

void
RenderZCircle(TiltedRenderer* renderer, Vec3 center, R32 radius, U32 color);

void
RenderZLineCircle(TiltedRenderer* renderer, Vec3 center, R32 radius, R32 line_width, U32 color);