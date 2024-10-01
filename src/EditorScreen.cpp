#include <imgui.h>
#include <gl3w.h>

#include "AnymUtil.h"
#include "EditorScreen.h"
#include "SimulationSettings.h"
#include "Lucide.h"

#include "Agent.h"

int
UpdateEditorScreen(EditorScreen* editor, Window* window)
{
    InputHandler* input = &window->input;
    Camera2D* cam = &editor->cam;
    Renderer* renderer = editor->renderer;

    // Do ui 
    ImGui::BeginMainMenuBar();
    if(ImGui::BeginMenu("Window"))
    {
        if(ImGui::MenuItem("Back to simulation" ICON_LC_ARROW_BIG_LEFT))
        {
            global_settings.current_phase = GamePhase_SimulationScreen;
        }
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();

    ImGuiIO& imgui_io = ImGui::GetIO();
    if(!imgui_io.WantCaptureMouse)
    {
        UpdateCameraScrollInput(cam, input);
        UpdateCameraDragInput(cam, input);
    }

    Skeleton* skeleton = editor->skeleton;

    UpdateSkeleton(skeleton);
    RenderSkeleton(renderer, skeleton);

    // Render entire thing 
    Render(renderer, cam, window->width, window->height);

    return 0;
}

void
InitEditorScreen(EditorScreen* editor)
{
    // Might be a bit much memory for the editor.
    MemoryArena* arena = editor->editor_arena = CreateMemoryArena(MegaBytes(32));
    editor->cam.pos = V2(0,0);
    editor->cam.scale = 100;
    editor->renderer = CreateRenderer(arena);
    editor->skeleton = CreateSkeleton(arena, V3(0, 0,0), 1.0f);
}
