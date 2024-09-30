#pragma once

#include "Mesh2D.h"
#include "Shader.h"
#include "Window.h"
#include "Camera2D.h"

struct EditorScreen
{
    Mesh2D dynamic_mesh;
    MemoryArena* editor_arena;
    Camera2D cam;
    Shader shader;
};

int
UpdateEditorScreen(EditorScreen* editor, Window* window);

void
InitEditorScreen(EditorScreen* screen);