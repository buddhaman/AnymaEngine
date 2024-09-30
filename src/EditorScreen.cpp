#include <imgui.h>

#include "AnymUtil.h"
#include "EditorScreen.h"
#include "SimulationSettings.h"
#include "Lucide.h"

#include "Agent.h"

void
RenderAgentDetails(Agent* agent)
{
}

int
UpdateEditorScreen(EditorScreen* editor, Window* window)
{
    InputHandler* input = &window->input;
    Camera2D* cam = &editor->cam;
    Mesh2D* mesh = &editor->dynamic_mesh;

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

    Agent agent;
    agent.details = PushStruct(editor->editor_arena, AgentDetails);

    UseShader(&editor->shader);
    UpdateCamera(cam, window->width, window->height);
    SetTransform(&editor->shader, &editor->cam.transform.m[0][0]);

    PushCircularSector(mesh, V2(0,0), 10, 12, 0, (R32)M_PI, V2(0,0), RGBAColor(255, 0, 0, 255));
    BufferData(mesh, GL_DYNAMIC_DRAW);
    Draw(mesh);
    Clear(mesh);

    return 0;
}

void
InitEditorScreen(EditorScreen* editor)
{
    // Might be a bit much memory for the editor.
    editor->editor_arena = CreateMemoryArena(MegaBytes(32));
    editor->shader = CreateShader();
    editor->cam.pos = V2(0,0);
}