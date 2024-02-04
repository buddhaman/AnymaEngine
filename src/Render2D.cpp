#include <GL/glew.h>

#include "Render2D.h"

void
PushVertex(Mesh2D& mesh, Vec2 pos, Vec2 texcoords, Vec4 color)
{
    mesh.vertex_buffer.emplace_back(pos, texcoords, color);
}

void
PushIndex(Mesh2D& mesh, U32 index)
{
    mesh.index_buffer.emplace_back(index);
}

void
BufferData(Mesh2D& mesh)
{
    glBindVertexArray(mesh.vertex_array_handle);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vertex_buffer_handle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex2D)*mesh.vertex_buffer.size(), mesh.vertex_buffer.data(), GL_STATIC_DRAW);
}

void
Draw(Mesh2D& mesh)
{
    glBindVertexArray(mesh.vertex_array_handle);
    glDrawArrays(GL_TRIANGLES, 0, mesh.vertex_buffer.size());
}

Mesh2D::Mesh2D()
{
    glGenVertexArrays(1, &vertex_array_handle);
    glGenBuffers(1, &vertex_buffer_handle);
    U32 stride = sizeof(Vertex2D);

    glBindVertexArray(vertex_array_handle);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_handle);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)offsetof(Vertex2D, pos));
    glEnableVertexAttribArray(0);

    // Texture attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*)offsetof(Vertex2D, texture_coords));
    glEnableVertexAttribArray(1);

    // Color attribute
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, (GLvoid*)offsetof(Vertex2D, color));
    glEnableVertexAttribArray(2);

    // Unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}