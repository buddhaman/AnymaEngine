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
PushVertex(Mesh2D* mesh, Vec2 pos, Vec2 texture_coords, U32 color);

void
PushIndex(Mesh2D* mesh, U32 index);

void
PushQuad(Mesh2D* mesh, 
         Vec2 p0, Vec2 uv0, 
         Vec2 p1, Vec2 uv1, 
         Vec2 p2, Vec2 uv2, 
         Vec2 p3, Vec2 uv3, U32 color);

void
PushQuad(Mesh2D* mesh, Vec2 p0, Vec2 p1, Vec2 p2, Vec2 p3, Vec2 u_orig, Vec2 u_size, U32 color);

void
PushRect(Mesh2D* mesh, Vec2 pos, Vec2 dims, Vec2 u_orig, Vec2 u_size, U32 color);

void
PushLine(Mesh2D *mesh, Vec2 from, Vec2 to, R32 line_width, Vec2 u_orig, Vec2 u_size, U32 color);

void
PushTrapezoid(Mesh2D *mesh, Vec2 from, R32 from_width, Vec2 to, R32 to_width, Vec2 u_orig, Vec2 u_size, U32 color);

void
PushLineRect(Mesh2D* mesh, Vec2 pos, Vec2 dims, R32 line_width, Vec2 u_orig, Vec2 u_size, U32 color);

// The u_orig texture coord does not really make any sense here.
void
PushCircularSector(Mesh2D* mesh, Vec2 center, R32 radius, int n, R32 min_angle, R32 max_angle, Vec2 u_orig, U32 color);

void
PushNGon(Mesh2D *mesh, Vec2 pos, R32 radius, I32 n, R32 orientation, Vec2 u_orig, U32 color);

void
PushLineNGon(Mesh2D *mesh, Vec2 pos, R32 inner_radius, R32 outer_radius, int n, R32 orientation, Vec2 u_orig, U32 color);

void
PushNGon(Mesh2D *mesh, Vec2 pos, I32 n, Vec2 axis0, Vec2 axis1, Vec2 u_orig, U32 color);

void
PushLineNGon(Mesh2D *mesh, Vec2 center, R32 inner_radius, R32 outer_radius, int n, Vec2 axis0, Vec2 axis1, Vec2 u_orig, U32 color);

void
BufferData(Mesh2D* mesh, U32 drawMode);

void
Draw(Mesh2D* mesh);

void
Clear(Mesh2D* mesh);