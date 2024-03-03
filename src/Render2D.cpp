#include <GL/glew.h>

#include "Render2D.h"

void
PushVertex(Mesh2D& mesh, Vec2 pos, Vec2 texture_coords, U32 color)
{
    Vertex2D* vert = mesh.vertex_buffer.PushBack();
    vert->pos = pos;
    vert->texture_coords = texture_coords;
    vert->color = color;
}

void
PushIndex(Mesh2D& mesh, U32 index)
{
    mesh.index_buffer.PushBack(index);
}

void
PushQuad(Mesh2D& mesh, 
         Vec2 p0, Vec2 uv0, 
         Vec2 p1, Vec2 uv1, 
         Vec2 p2, Vec2 uv2, 
         Vec2 p3, Vec2 uv3, U32 color)
{
    U32 lastIndex = mesh.vertex_buffer.size;

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
BufferData(Mesh2D& mesh, GLenum drawMode)
{
    glBindVertexArray(mesh.vertex_array_handle);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vertex_buffer_handle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex2D)*mesh.vertex_buffer.size, mesh.vertex_buffer.data, drawMode);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.element_buffer_handle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Vertex2D)*mesh.index_buffer.size, mesh.index_buffer.data, drawMode);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void
Draw(Mesh2D& mesh)
{
    glBindVertexArray(mesh.vertex_array_handle);
    glDrawElements(GL_TRIANGLES, mesh.index_buffer.size, GL_UNSIGNED_INT, nullptr);
}

void
Clear(Mesh2D& mesh)
{
    mesh.index_buffer.Clear();
    mesh.vertex_buffer.Clear();
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)offsetof(Vertex2D, pos));
    glEnableVertexAttribArray(0);

    // Texture attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*)offsetof(Vertex2D, texture_coords));
    glEnableVertexAttribArray(1);

    // Color attribute
    glVertexAttribPointer(2, 4, GL_BYTE, GL_TRUE, stride, (GLvoid*)offsetof(Vertex2D, color));
    glEnableVertexAttribArray(2);

    // Unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}