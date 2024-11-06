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
    TiltedRenderer* renderer = editor->renderer;

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

    R32 tilt_speed = -0.01f;
    if(IsKeyDown(input, InputAction_W))
    {
        renderer->cam.angle += tilt_speed;
    }
    if(IsKeyDown(input, InputAction_S))
    {
        renderer->cam.angle -= tilt_speed;
    }

    renderer->cam.angle = Clamp(-PI_R32/2.0f+0.001f, renderer->cam.angle, 0.0f);

    UpdateTiltedCamera(&editor->renderer->cam, window->width, window->height);

    ImGuiIO& imgui_io = ImGui::GetIO();
    if(!imgui_io.WantCaptureMouse)
    {
        UpdateTiltedCameraScrollInput(&editor->renderer->cam, input);
        UpdateTiltedCameraDragInput(&editor->renderer->cam, input);
    }

    Skeleton* skeleton = editor->skeleton;

    UpdateSkeleton(skeleton);
    RenderSkeleton(renderer, skeleton);

    // Render entire thing 
    Render(renderer->renderer, cam, window->width, window->height);

    return 0;
}

Leg* 
AddLeg(Skeleton* skele, int idx_in_body, R32 r, U32 color)
{
    Leg* leg = skele->legs.PushBack();
    int body_particle_idx = skele->body.body[idx_in_body];
    Joint* attach_to = &skele->joints[body_particle_idx];
    Vec3 ground_diff = V3(0,0,attach_to->v->pos.z);
    AddJoint(skele, attach_to->v->pos-0.5f*ground_diff, r, color);
    AddJoint(skele, attach_to->v->pos-1.0f*ground_diff, r, color);
    leg->idx = skele->joints.size-1;
    Connect(skele, body_particle_idx, r*2, leg->idx-1, r*2, color);
    Connect(skele, leg->idx-1, r*2, leg->idx, r*2, color);
    return leg;
}

void
InitEditorScreen(EditorScreen* editor)
{
    // Might be a bit much memory for the editor.
    MemoryArena* arena = editor->editor_arena = CreateMemoryArena(MegaBytes(32));
    editor->cam.pos = V2(0,0);
    editor->cam.scale = 1;
    editor->renderer = CreateTiltedRenderer(arena);
    editor->renderer->cam.angle = -PI_R32/4.0f;
    editor->genes = CreateGenes(arena, 24);
    int max_depth = 24;
    Skeleton* skele = editor->skeleton = CreateSkeleton(arena, max_depth, max_depth*2);

#if 0
    R32 hue = 0.0f;
    U32 color = HSVAToRGBA(hue, 1.0f, 1.0f, 1.0f);
    Vec3 pos = V3(0,0,0);
    AddJoint(skele, pos, 1.0f, color);
    for(int i = 1; i < max_depth; i++)
    {
        U32 color = HSVAToRGBA(hue, 1.0f, 1.0f, 1.0f);
        R32 r0 = 1.0f + (i-1) / 10.0f;
        R32 r1 = 1.0f + i / 10.0f;
        hue += 6;
        pos += V3(4,0,0);
        AddJoint(skele, pos, r1, color);
        Connect(skele, i-1, r0*2, i, r1*2, color);
    }
#else

    int n_legs = 3;
    U32 color = Color_White;
    R32 scale = 1.0f;

    skele->legs = CreateArray<Leg>(arena, n_legs);

    // Start with head
    Vec3 pos = V3(0,0,scale*4);

    // Head
    AddJoint(skele, pos, scale*2, color);
    skele->head.idx = 0;
    skele->head.target_position = pos;

    // Body
    R32 diff = 4.0f*scale;
    skele->body.body = CreateArray<int>(arena, n_legs);
    skele->body.target_positions = CreateArray<Vec3>(arena, n_legs);
    for(int i = 0; i < n_legs; i++)
    {
        pos += V3(diff, 0, 0);

        AddJoint(skele, pos, scale, color);
        int idx = skele->particles.size-1;
        skele->body.body.PushBack(idx);
        skele->body.target_positions.PushBack(pos);

        int prev_idx = i==0? skele->head.idx : idx-1;
        Connect(skele, prev_idx, scale*2, idx, scale*2, color);
    }

    AddLeg(skele, 0, 1.0f, color);
    AddLeg(skele, 1, 1.0f, color);
    AddLeg(skele, 2, 1.0f, color);

#endif

    // AddImpulse(&skele->particles[1], V3(0,2,0));
}
