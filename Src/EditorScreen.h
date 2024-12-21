#pragma once

#include "Mesh2D.h"
#include "Shader.h"
#include "Window.h"
#include "Camera2D.h"
#include "Texture.h"
#include "Renderer.h"

struct EditorScreen
{
    MemoryArena* editor_arena;
    Camera2D cam;

    TextureAtlas* atlas;

    TiltedRenderer* renderer;

    Agent* agent;
    PhenoType* phenotype;

    R32 time = 0.0f;
};

int
UpdateEditorScreen(EditorScreen* editor, Window* window);

void
InitEditorScreen(EditorScreen* screen);
