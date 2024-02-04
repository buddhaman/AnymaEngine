#pragma once

#include <vector>

#include "AnymUtil.h"

struct Vertex2D
{
    Vec2 pos;
    Vec2 texture_coords;
    Vec4 color;
};

struct Mesh2D
{
    std::vector<Vertex2D> vertex_buffer;
    std::vector<U32> index_buffer;

    U32 vertex_buffer_handle;
    U32 vertex_array_handle;

    Mesh2D();
};

void
PushVertex(Mesh2D& mesh, Vec2 pos, Vec2 texcoords, Vec4 color);

void
PushIndex(Mesh2D& mesh, U32 index);

void
BufferData(Mesh2D& mesh);

void
Draw(Mesh2D& mesh);