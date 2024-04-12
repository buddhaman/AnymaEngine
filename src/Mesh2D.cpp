#include <GL/glew.h>

#include "Mesh2D.h"

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
    U32 lastIndex = mesh->vertex_buffer.size;

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
    R32 invl = 1.0/V2Len(diff);
    Vec2 perp = V2(diff.y*invl*line_width/2.0, -diff.x*invl*line_width/2.0);

    PushQuad(mesh,
            V2Add(from, perp), 
            V2Add(to, perp),
            V2Sub(to, perp), 
            V2Sub(from, perp),
            u_orig, u_size, color);
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
PushNGon(Mesh2D *mesh,
        Vec2 pos,
        R32 radius, 
        I32 n,
        R32 orientation,
        Vec2 u_orig, 
        U32 color)
{
    I32 npoints = n+1;
    U32 last_idx = mesh->vertex_buffer.size;
    R32 angdiff = 2*M_PI/(npoints-1);
    PushVertex(mesh, pos, u_orig, color);
    for(U32 atPoint = 0;
            atPoint < npoints;
            atPoint++)
    {
        R32 angle = orientation+angdiff*atPoint;
        Vec2 point = V2Add(pos, V2Polar(angle, radius));
        PushVertex(mesh, point, u_orig, color);
    }
    for(U32 at_point = 0;
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
        I32 n,
        Vec2 axis0, 
        Vec2 axis1, 
        Vec2 u_orig, 
        U32 color)
{
    I32 npoints = n+1;
    U32 last_idx = mesh->vertex_buffer.size;
    R32 angdiff = 2*M_PI/(npoints-1);
    PushVertex(mesh, pos, u_orig, color);
    for(U32 atPoint = 0;
            atPoint < npoints;
            atPoint++)
    {
        R32 angle = angdiff*atPoint;
        R32 s = sinf(angle);
        R32 c = cosf(angle);
        Vec2 point = pos + axis0*c + axis1*s;
        PushVertex(mesh, point, u_orig, color);
    }
    for(U32 at_point = 0;
            at_point < npoints-1;
            at_point++)
    {
        PushIndex(mesh, last_idx);
        PushIndex(mesh, last_idx+at_point+1);
        PushIndex(mesh, last_idx+at_point+2);
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
    glDrawElements(GL_TRIANGLES, mesh->index_buffer.size, GL_UNSIGNED_INT, nullptr);
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