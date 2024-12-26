#include "Renderer.h"

void
RenderLine(Renderer* renderer, Vec2 from, Vec2 to, R32 line_width, U32 color)
{
    PushLine(&renderer->mesh, from, to, line_width, renderer->square->pos, V2(0,0), color);
}

void
RenderTrapezoid(Renderer* renderer, Vec2 from, R32 from_width, Vec2 to, R32 to_width, U32 color)
{
    PushTrapezoid(&renderer->mesh, from, from_width, to, to_width, renderer->square->pos, V2(0,0), color);
}

void
RenderCircle(Renderer* renderer, Vec2 center, R32 radius, U32 color)
{
    PushRect(&renderer->mesh, 
             V2(center.x - radius, center.y - radius), 
             V2(radius*2, radius*2), 
             renderer->circle->pos, renderer->circle->size, 
             color);
}

void
RenderRect(Renderer* renderer, Vec2 center, Vec2 dims, AtlasRegion* region, U32 color)
{
    PushRect(&renderer->mesh, 
             V2(center.x - dims.x*0.5f, center.y - dims.y*0.5f), 
             dims,
             region->pos, region->size,
             color);
}

void
Render(Renderer* renderer, Camera2D* cam, int width, int height)
{
    UseShader(&renderer->shader);

    glBindTexture(GL_TEXTURE_2D, renderer->atlas->handle);

    UpdateCamera(cam, width, height);
    SetTransform(&renderer->shader, &cam->transform.m[0][0]);
    BufferData(&renderer->mesh, GL_DYNAMIC_DRAW);
    Draw(&renderer->mesh);
    Clear(&renderer->mesh);
}

Renderer* 
CreateRenderer(MemoryArena* arena)
{
    Renderer* renderer = PushNewStruct(arena, Renderer);

    // Atlas
    renderer->atlas = MakeDefaultTexture(arena, 511);
    renderer->circle = &renderer->atlas->regions[0];
    renderer->square = &renderer->atlas->regions[1];

    // Shader
    renderer->shader = CreateShader();

    return renderer;
}

TiltedRenderer* 
CreateTiltedRenderer(MemoryArena* arena)
{
    TiltedRenderer* renderer = PushNewStruct(arena, TiltedRenderer);
    renderer->renderer = CreateRenderer(arena);
    return renderer;
}

void
RenderLine(TiltedRenderer* renderer, Vec3 from, Vec3 to, R32 line_width, U32 color)
{
    RenderLine(renderer->renderer,
             GetVec2(&renderer->cam, from), 
             GetVec2(&renderer->cam, to), 
             GetScaled(&renderer->cam, line_width), 
             color);
}

void
RenderTrapezoid(TiltedRenderer* renderer, Vec3 from, R32 from_width, Vec3 to, R32 to_width, U32 color)
{
    RenderTrapezoid(renderer->renderer, 
                GetVec2(&renderer->cam, from), 
                GetScaled(&renderer->cam, from_width), 
                GetVec2(&renderer->cam, to), 
                GetScaled(&renderer->cam, to_width), 
                color);
}

void
RenderCircle(TiltedRenderer* renderer, Vec3 center, R32 radius, U32 color)
{
    RenderCircle(renderer->renderer, 
                 GetVec2(&renderer->cam, center), 
                 GetScaled(&renderer->cam, radius), color);
}

void
RenderZRect(TiltedRenderer* renderer, Vec3 center, Vec2 dims, AtlasRegion* region, U32 color)
{
    RenderRect(renderer->renderer, 
                GetVec2(&renderer->cam, center),
                V2(GetScaled(&renderer->cam, dims.x), GetScaled(&renderer->cam, renderer->cam.c*dims.y)),
                region, color);
}

void
RenderZRect(TiltedRenderer* renderer, Vec3 center, Vec2 dims, AtlasRegion* region, 
            U32 color0, U32 color1, U32 color2, U32 color3)
{
    Vec2 scaledDims = V2(GetScaled(&renderer->cam, dims.x), 
                         GetScaled(&renderer->cam, renderer->cam.c * dims.y));
    Vec2 baseCenter = GetVec2(&renderer->cam, center);

    Vec2 p0 = V2(baseCenter.x - scaledDims.x * 0.5f, baseCenter.y - scaledDims.y * 0.5f);
    Vec2 p1 = V2(baseCenter.x + scaledDims.x * 0.5f, baseCenter.y - scaledDims.y * 0.5f);
    Vec2 p2 = V2(baseCenter.x + scaledDims.x * 0.5f, baseCenter.y + scaledDims.y * 0.5f);
    Vec2 p3 = V2(baseCenter.x - scaledDims.x * 0.5f, baseCenter.y + scaledDims.y * 0.5f);

    Vec2 uv0 = region->pos;
    Vec2 uv1 = V2(region->pos.x + region->size.x, region->pos.y);
    Vec2 uv2 = V2(region->pos.x + region->size.x, region->pos.y + region->size.y);
    Vec2 uv3 = V2(region->pos.x, region->pos.y + region->size.y);

    PushQuad(&renderer->renderer->mesh, 
             p0, uv0, color0,
             p1, uv1, color1,
             p2, uv2, color2,
             p3, uv3, color3);
}

void
RenderYRect(TiltedRenderer* renderer, Vec3 center, Vec2 dims, AtlasRegion* region, U32 color)
{
    RenderRect(renderer->renderer, 
                GetVec2(&renderer->cam, center),
                V2(GetScaled(&renderer->cam, dims.x), GetScaled(&renderer->cam, renderer->cam.s*dims.y)),
                region, color);
}

void
RenderZCircle(TiltedRenderer* renderer, Vec3 center, R32 radius, U32 color)
{
    RenderZRect(renderer, center, V2(radius*2, radius*2), renderer->renderer->circle, color);
}

void
RenderZLineCircle(TiltedRenderer* renderer, Vec3 center, R32 radius, R32 line_width, U32 color)
{
    PushLineNGon(&renderer->renderer->mesh, 
                 GetVec2(&renderer->cam, center), 
                 GetScaled(&renderer->cam, radius-line_width/2.0f),
                 GetScaled(&renderer->cam, radius+line_width/2.0f),
                 24,
                 V2(1, 0),
                 V2(0, renderer->cam.c),
                 renderer->renderer->square->pos,
                 color);
}