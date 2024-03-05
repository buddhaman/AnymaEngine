#pragma once

#include <vector>

#include "AnymUtil.h"
#include "Array.h"

struct Vertex2D
{
    Vec2 pos;
    Vec2 texture_coords;
    U32 color;
};

struct Mesh2D
{
    DynamicArray<Vertex2D> vertex_buffer;
    DynamicArray<U32> index_buffer;

    U32 vertex_buffer_handle;
    U32 vertex_array_handle;
    U32 element_buffer_handle;

    Mesh2D();
    ~Mesh2D() = default;
};

void
PushQuad(Mesh2D* mesh, 
         Vec2 p0, Vec2 uv0, 
         Vec2 p1, Vec2 uv1, 
         Vec2 p2, Vec2 uv2, 
         Vec2 p3, Vec2 uv3, U32 color);

void
PushRect(Mesh2D* mesh, Vec2 pos, Vec2 dims, Vec2 tex_offset, Vec2 tex_extent, U32 color);

void
PushVertex(Mesh2D* mesh, Vec2 pos, Vec2 texture_coords, U32 color);

void
PushIndex(Mesh2D* mesh, U32 index);

void
BufferData(Mesh2D* mesh, U32 drawMode);

void
Draw(Mesh2D* mesh);

void
Clear(Mesh2D* mesh);