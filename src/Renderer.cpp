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