#include <imgui.h>
#include <gl3w.h>

#include "AnymUtil.h"
#include "EditorScreen.h"
#include "SimulationSettings.h"
#include "Lucide.h"

#include "Agent.h"

void
UpdateAgentSkeleton(Agent* agent)
{
    // TEST:
    R32 speed = 0.12f;
    Vec2 direction = V2Polar(agent->orientation, 1.0f);
    agent->pos += direction * speed;

    agent->orientation += 0.002f;

    Vec3 center;
    center.xy = agent->pos;
    center.z = 0.0f;

    MainBody* body = &agent->body;
    Skeleton* skeleton = agent->skeleton;
    R32 body_force = 0.4f;
    for(int bodypart_idx : body->body)
    {
        Joint* j = &skeleton->joints[bodypart_idx];
        AddImpulse(j->v, V3(0,0,body_force));
    }

    // Put all feet on the ground.
    for(Leg& leg : agent->legs)
    {
        Joint* j = &skeleton->joints[leg.idx];
        j->v->pos.z = 0;
        j->v->pos = XForm(center, direction, leg.offset);
    }

    // Move head up and set X,Y to proper agent x, y position
    Joint* head = &skeleton->joints[agent->head.idx];
    head->v->pos.xy = agent->pos;
    AddImpulse(head->v, V3(0,0,body_force));
}

int
UpdateEditorScreen(EditorScreen* editor, Window* window)
{
    InputHandler* input = &window->input;
    Camera2D* cam = &editor->cam;
    TiltedRenderer* renderer = editor->renderer;

    Agent* agent = editor->agent;

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

    Skeleton* skeleton = agent->skeleton;

    UpdateSkeleton(skeleton);
    UpdateAgentSkeleton(agent);
    RenderSkeleton(renderer, skeleton);

    // Debug rendering
    Vec2 direction = V2Polar(agent->orientation, 1.0f);
    Vec3 agent_pos = V3(agent->pos.x, agent->pos.y, 0);
    RenderCircle(renderer, agent_pos, 0.1f, Color_Red);
    for(Leg& leg : agent->legs)
    {
        RenderCircle(renderer, XForm(agent_pos, direction, leg.offset), 0.1f, Color_Green);
    }

    // Render entire thing 
    Render(renderer->renderer, cam, window->width, window->height);

    return 0;
}

Leg* 
AddLeg(Agent* agent, int idx_in_body, int dir, R32 r, U32 color)
{
    Skeleton* skele = agent->skeleton;
    Leg* leg = agent->legs.PushBack();
    int body_particle_idx = agent->body.body[idx_in_body];
    Joint* attach_to = &skele->joints[body_particle_idx];
    Vec3 ground_diff = V3(0,0,attach_to->v->pos.z);
    AddJoint(skele, attach_to->v->pos-0.5f*ground_diff, r, color);
    AddJoint(skele, attach_to->v->pos-1.0f*ground_diff, r, color);

    // set idx to foot.
    leg->idx = (int)skele->joints.size-1;
    leg->offset.xy = agent->body.target_positions[idx_in_body].xy;
    leg->offset.y += dir*5;
    leg->offset.z = 0.0f;

    Connect(skele, body_particle_idx, r*2, leg->idx-1, r*2, color);
    Connect(skele, leg->idx-1, r*2, leg->idx, r*2, color);

    return leg;
}

void 
InitAgentSkeleton(MemoryArena* arena, Agent* agent)
{
    int n_legs = 3;
    U32 color = Color_Brown;
    R32 scale = 1.0f;
    Skeleton* skele = agent->skeleton;

    agent->legs = CreateArray<Leg>(arena, n_legs*2);

    // Start with head
    Vec3 pos = V3(0,0,scale*4);

    // Head
    AddJoint(skele, pos, scale*2, color);
    agent->head.idx = 0;
    agent->head.target_position = pos;

    // Body
    R32 diff = 4.0f*scale;
    agent->body.body = CreateArray<int>(arena, n_legs);
    agent->body.target_positions = CreateArray<Vec3>(arena, n_legs);
    for(int i = 0; i < n_legs; i++)
    {
        pos += V3(diff, 0, 0);

        AddJoint(skele, pos, scale, color);
        int idx = (int)skele->particles.size-1;
        agent->body.body.PushBack(idx);
        agent->body.target_positions.PushBack(pos);

        int prev_idx = i==0? agent->head.idx : idx-1;
        Connect(skele, prev_idx, scale*2, idx, scale*2, color);
    }

    AddLeg(agent, 0, -1, 1.0f, color);
    AddLeg(agent, 0, +1, 1.0f, color);
    AddLeg(agent, 1, -1, 1.0f, color);
    AddLeg(agent, 1, +1, 1.0f, color);
    AddLeg(agent, 2, -1, 1.0f, color);
    AddLeg(agent, 2, +1, 1.0f, color);
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
    int max_joints = 24;
    editor->agent = PushNewStruct(arena, Agent);
    editor->agent->skeleton = CreateSkeleton(arena, max_joints, max_joints*2);
    InitAgentSkeleton(arena, editor->agent);

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

#endif

    // AddImpulse(&skele->particles[1], V3(0,2,0));
}
