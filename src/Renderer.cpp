#include "Renderer.h"

void
RenderLine(Renderer* renderer, Vec2 from, Vec2 to, R32 line_width, U32 color)
{
    PushLine(&renderer->mesh, from, to, line_width, renderer->square->pos, V2(0,0), color);
}

void
RenderCircle(Renderer* renderer, Vec2 center, R32 radius, U32 color)
{
    PushRect(&renderer->mesh, V2(center.x - radius, center.y - radius), V2(radius, radius), renderer->circle->pos, renderer->circle->size, color);
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