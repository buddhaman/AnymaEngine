#include "Mesh2D.h"
#include <gl3w.h>

void
PushVertex(Mesh2D* mesh, Vec2 pos, Vec2 texture_coords, U32 color)
{
    Vertex2D* vert = mesh->vertex_buffer.PushBack();
    vert->pos = pos;
    vert->texture_coords = texture_coords;
    vert->color = color;
}

void
PushIndex(Mesh2D* mesh, U32 index)
{
    mesh->index_buffer.PushBack(index);
}

void
PushQuad(Mesh2D* mesh, 
         Vec2 p0, Vec2 uv0, 
         Vec2 p1, Vec2 uv1, 
         Vec2 p2, Vec2 uv2, 
         Vec2 p3, Vec2 uv3, U32 color)
{
    U32 lastIndex = (U32)mesh->vertex_buffer.size;

    PushVertex(mesh, p0, uv0, color);
    PushVertex(mesh, p1, uv1, color);
    PushVertex(mesh, p2, uv2, color);
    PushVertex(mesh, p3, uv3, color);

    PushIndex(mesh, lastIndex);
    PushIndex(mesh, lastIndex+1);
    PushIndex(mesh, lastIndex+2);
    PushIndex(mesh, lastIndex+2);
    PushIndex(mesh, lastIndex+3);
    PushIndex(mesh, lastIndex);
}

void
PushQuad(Mesh2D* mesh, Vec2 p0, Vec2 p1, Vec2 p2, Vec2 p3, Vec2 u_orig, Vec2 u_size, U32 color)
{
    PushQuad(mesh, p0, u_orig, 
        p1, V2(u_orig.x+u_size.x, u_orig.y),
        p2, V2(u_orig.x+u_size.x, u_orig.y+u_size.y),
        p3, V2(u_orig.x, u_orig.y+u_size.y), color);
}

void
PushRect(Mesh2D* mesh, Vec2 pos, Vec2 dims, Vec2 u_orig, Vec2 u_size, U32 color)
{
    PushQuad(mesh, 
             pos, 
             V2(pos.x+dims.x, pos.y), 
             V2(pos.x+dims.x, pos.y+dims.y), 
             V2(pos.x, pos.y+dims.y), 
             u_orig, u_size,
             color);
}

void
PushLine(Mesh2D *mesh, Vec2 from, Vec2 to, R32 line_width, Vec2 u_orig, Vec2 u_size, U32 color)
{
    Vec2 diff = V2Sub(to, from);
    R32 invl = 1.0f/V2Len(diff);
    Vec2 perp = V2(diff.y*invl*line_width/2.0f, -diff.x*invl*line_width/2.0f);

    PushQuad(mesh,
            V2Add(from, perp), 
            V2Add(to, perp),
            V2Sub(to, perp), 
            V2Sub(from, perp),
            u_orig, u_size, color);
}

void
PushTrapezoid(Mesh2D *mesh, Vec2 from, R32 from_width, Vec2 to, R32 to_width, Vec2 u_orig, Vec2 u_size, U32 color)
{
    Vec2 diff = V2Sub(to, from);
    R32 invl = 1.0f/V2Len(diff);
    Vec2 perp = V2(diff.y*invl, -diff.x*invl);
    R32 fromhw = from_width/2.0f;
    R32 tohw = to_width/2.0f;

    PushQuad(mesh, from + perp*fromhw, to + perp*tohw, to - perp*tohw, from - perp*fromhw, u_orig, u_size, color);
}

void
PushLineRect(Mesh2D* mesh, Vec2 pos, Vec2 dims, R32 line_width, Vec2 u_orig, Vec2 u_size, U32 color)
{
    PushRect(mesh, 
             V2(pos.x - 0.5f*line_width, pos.y - 0.5f*line_width), 
             V2(dims.x, line_width), 
             u_orig, V2(0,0), color);

    PushRect(mesh, 
             V2(pos.x + dims.x - 0.5f*line_width, pos.y - 0.5f*line_width), 
             V2(line_width, dims.y), 
             u_orig, V2(0,0), color);

    PushRect(mesh, 
             V2(pos.x - 0.5f*line_width, pos.y + dims.y - 0.5f*line_width), 
             V2(dims.x+line_width, line_width), 
             u_orig, V2(0,0), color);

    PushRect(mesh, 
             V2(pos.x - 0.5f*line_width, pos.y - 0.5f*line_width), 
             V2(line_width, dims.y), 
             u_orig, V2(0,0), color);
}

void
PushCircularSector(Mesh2D* mesh, Vec2 center, R32 radius, int n, R32 min_angle, R32 max_angle, Vec2 u_orig, U32 color)
{
    R32 angdiff = (max_angle - min_angle) / (n - 1);
    U32 last_idx = (U32)mesh->vertex_buffer.size;

    PushVertex(mesh, center, u_orig, color);

    for (I32 i = 0; i < n; ++i)
    {
        R32 angle = min_angle + angdiff * i;
        Vec2 point = V2Add(center, V2Polar(angle, radius));
        PushVertex(mesh, point, u_orig, color);
    }

    for (I32 i = 0; i < n - 1; ++i)
    {
        PushIndex(mesh, last_idx);         
        PushIndex(mesh, last_idx + i + 1); 
        PushIndex(mesh, last_idx + i + 2);
    }
}

void
PushNGon(Mesh2D *mesh,
        Vec2 pos,
        R32 radius, 
        I32 n,
        R32 orientation,
        Vec2 u_orig, 
        U32 color)
{
    I32 npoints = n+1;
    U32 last_idx = (U32)mesh->vertex_buffer.size;
    // TODO: Expand math library and make R32 PI constant.
    R32 angdiff = 2*(R32)M_PI/(npoints-1);
    PushVertex(mesh, pos, u_orig, color);
    for(int atPoint = 0;
            atPoint < npoints;
            atPoint++)
    {
        R32 angle = orientation+angdiff*atPoint;
        Vec2 point = V2Add(pos, V2Polar(angle, radius));
        PushVertex(mesh, point, u_orig, color);
    }
    for(int at_point = 0;
            at_point < npoints-1;
            at_point++)
    {
        PushIndex(mesh, last_idx);
        PushIndex(mesh, last_idx+at_point+1);
        PushIndex(mesh, last_idx+at_point+2);
    }
}

void
PushNGon(Mesh2D *mesh,
        Vec2 pos,
        int n,
        Vec2 axis0, 
        Vec2 axis1, 
        Vec2 u_orig, 
        U32 color)
{
    int npoints = n+1;
    int last_idx = (int)mesh->vertex_buffer.size;
    R32 angdiff = 2*(R32)M_PI/(npoints-1);
    PushVertex(mesh, pos, u_orig, color);
    for(int atPoint = 0;
            atPoint < npoints;
            atPoint++)
    {
        R32 angle = angdiff*atPoint;
        R32 s = sinf(angle);
        R32 c = cosf(angle);
        Vec2 point = pos + axis0*c + axis1*s;
        PushVertex(mesh, point, u_orig, color);
    }
    for(int at_point = 0;
            at_point < npoints-1;
            at_point++)
    {
        PushIndex(mesh, last_idx);
        PushIndex(mesh, last_idx+at_point+1);
        PushIndex(mesh, last_idx+at_point+2);
    }
}

void 
PushLineNGon(Mesh2D *mesh, Vec2 center, R32 inner_radius, R32 outer_radius, int n, R32 orientation, Vec2 u_orig, U32 color)
{
    int npoints = n + 1;

    int inner_start_idx = (int)mesh->vertex_buffer.size;
    int outer_start_idx = inner_start_idx + npoints;

    R32 angdiff = 2 * (R32)M_PI / (npoints - 1);

    // Generate vertices for the inner N-gon
    for (int i = 0; i < npoints; ++i)
    {
        R32 angle = angdiff * i + orientation;
        R32 s = sinf(angle);
        R32 c = cosf(angle);

        Vec2 point = center + V2(c, s) * inner_radius;
        PushVertex(mesh, point, u_orig, color);
    }

    // Generate vertices for the outer N-gon
    for (int i = 0; i < npoints; ++i)
    {
        R32 angle = angdiff * i + orientation;
        R32 s = sinf(angle);
        R32 c = cosf(angle);

        Vec2 point = center + V2(c, s) * outer_radius;
        PushVertex(mesh, point, u_orig, color);
    }

    // Generate indices to form triangles between the inner and outer N-gons
    for (int i = 0; i < npoints - 1; ++i)
    {
        // First triangle
        PushIndex(mesh, inner_start_idx + i);
        PushIndex(mesh, outer_start_idx + i);
        PushIndex(mesh, outer_start_idx + i + 1);

        // Second triangle 
        PushIndex(mesh, inner_start_idx + i);
        PushIndex(mesh, outer_start_idx + i + 1);
        PushIndex(mesh, inner_start_idx + i + 1);
    }
}

void
BufferData(Mesh2D* mesh, U32 drawMode)
{
    glBindVertexArray(mesh->vertex_array_handle);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer_handle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex2D)*mesh->vertex_buffer.size, mesh->vertex_buffer.data, drawMode);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->element_buffer_handle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(U32)*mesh->index_buffer.size, mesh->index_buffer.data, drawMode);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void
Draw(Mesh2D* mesh)
{
    glBindVertexArray(mesh->vertex_array_handle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->element_buffer_handle); 
    glDrawElements(GL_TRIANGLES, (int)mesh->index_buffer.size, GL_UNSIGNED_INT, nullptr);
}

void
Clear(Mesh2D* mesh)
{
    mesh->index_buffer.Clear();
    mesh->vertex_buffer.Clear();
}

Mesh2D::Mesh2D()
{
    glGenVertexArrays(1, &vertex_array_handle);
    glGenBuffers(1, &vertex_buffer_handle);
    glGenBuffers(1, &element_buffer_handle);

    U32 stride = sizeof(Vertex2D);

    glBindVertexArray(vertex_array_handle);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_handle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_handle);
    
    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*)offsetof(Vertex2D, pos));
    glEnableVertexAttribArray(0);

    // Texture attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*)offsetof(Vertex2D, texture_coords));
    glEnableVertexAttribArray(1);

    // Color attribute
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (GLvoid*)offsetof(Vertex2D, color));
    glEnableVertexAttribArray(2);

    // Unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}