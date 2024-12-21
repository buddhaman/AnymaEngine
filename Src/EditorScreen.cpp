#include <imgui.h>
#include <gl3w.h>

#include "AnymUtil.h"
#include "EditorScreen.h"
#include "SimulationSettings.h"
#include "Lucide.h"

#include "Agent.h"

constexpr R32 speed = 0.34f;
constexpr R32 turn_speed = 0.01f;
constexpr int max_joints = 64;
static R32 walk_radius = speed/sinf(turn_speed);

void
InitRandomPhenotype(PhenoType* pheno)
{
    pheno->n_backbones = RandomIntDebug(1, 10);

    // Bettter make differnt distribution, technically this can go below 0.
    pheno->backbone_radius.Set(0);
    pheno->backbone_radius.AddNormal(3.0f, 1.0f);

    pheno->knee_size.Set(0.0f);
    pheno->knee_size.AddNormal(1.0f, 0.5f);

    pheno->foot_size.Set(0.0f);
    pheno->foot_size.AddNormal(1.0f, 0.5f);

    pheno->step_radius.Set(0.0f);
    pheno->step_radius.AddNormal(1.0f, 0.25f);

    for(int i = 0; i < pheno->n_backbones; i++)
    {
        pheno->has_leg[i] = RandomR32Debug(0, 1) < 0.5f;
    }

    pheno->elbow_size = 1.0f;
    pheno->hand_size = 1.0f;

    pheno->color = Vec4ToColor(V4(
        RandomR32Debug(0.0f, 1.0f),
        RandomR32Debug(0.0f, 1.0f),
        RandomR32Debug(0.0f, 1.0f),
        1.0f
    ));
}

void
UpdateAgentSkeleton(Agent* agent)
{
    Vec3 direction = V3(0,0,0);
    direction.xy = V2Polar(agent->orientation, 1.0f);
    agent->pos += direction.xy * speed;
    Vec3 perp = V3(-direction.y, direction.x, 0);

    // agent->orientation += RandomR32Debug(-turn_speed, turn_speed);
    agent->orientation += turn_speed;

    Vec3 center;
    center.xy = agent->pos;
    center.z = 0.0f;

    MainBody* body = &agent->body;
    Skeleton* skeleton = agent->skeleton;

    R32 body_force = (agent->legs.size==0) ? -0.4f : 0.4f;
    for(int bodypart_idx : body->body)
    {
        Joint* j = &skeleton->joints[bodypart_idx];
        AddImpulse(j->v, V3(0,0,body_force));
    }

    for(Verlet3& p : skeleton->particles)
    {
        if(p.pos.z < 0.0f) p.pos.z = 0.0f;
    }

    for(Leg& leg : agent->legs)
    {
        Joint* j = &skeleton->joints[leg.idx];
        
        Vec3 world_target = XForm(center, direction.xy, leg.target_offset);

        // distance of foot to target. If more than r then move foot.
        R32 feet_diff = V3Len(world_target - leg.foot_pos);
        if(feet_diff > leg.r)
        {
            leg.foot_pos = world_target;
            leg.foot_pos.xy += direction.xy * RandomR32Debug(0.0f, leg.r);
        }

        j->v->pos.z = 0;
        j->v->pos = leg.foot_pos;
    }

    // Update arms
    Joint* l_arm = &skeleton->joints[agent->l_arm.idx];
    AddImpulse(l_arm->v, perp*0.5f);
    Joint* r_arm = &skeleton->joints[agent->r_arm.idx];
    AddImpulse(r_arm->v, -perp*0.5f);

    // Move head up and set X,Y to proper agent x, y position
    Joint* head = &skeleton->joints[agent->head.idx];

    // Only takes into account agents head X position. But that should work.
    head->v->pos.xy = agent->pos + V2Polar(agent->orientation, agent->head.target_position.x);

    AddImpulse(head->v, V3(0,0,body_force));
}

Leg* 
InitLeg(Agent* agent, Leg* leg, int idx_in_body, int dir, R32 mid_radius, R32 end_radius, U32 color)
{
    Skeleton* skele = agent->skeleton;
    int body_particle_idx = agent->body.body[idx_in_body];
    Joint* attach_to = &skele->joints[body_particle_idx];
    Vec3 ground_diff = V3(0,0,attach_to->v->pos.z);
    R32 back_size = agent->phenotype->backbone_radius[idx_in_body];
    AddJoint(skele, attach_to->v->pos-0.5f*ground_diff, mid_radius, color);
    AddJoint(skele, attach_to->v->pos-1.0f*ground_diff, end_radius, color);

    // set idx to foot.
    leg->idx = (int)skele->joints.size-1;
    leg->target_offset.xy = agent->body.target_positions[idx_in_body].xy;
    leg->target_offset.y += dir*3;
    leg->target_offset.z = 0.0f;
    leg->r = agent->phenotype->step_radius[idx_in_body];

    Connect(skele, body_particle_idx, back_size*2, leg->idx-1, mid_radius*2, color);
    Connect(skele, leg->idx-1, mid_radius*2, leg->idx, end_radius*2, color);

    return leg;
}

void 
InitAgentSkeleton(MemoryArena* arena, Agent* agent)
{
    PhenoType* phenotype = agent->phenotype;
    Skeleton* skele = agent->skeleton;

    int n_backbones = phenotype->n_backbones;
    U32 color = phenotype->color;
    R32 scale = 1.0f;

    agent->legs = CreateArray<Leg>(arena, n_backbones*2);

    // Start with neck
    Vec3 pos = V3(0,0,scale*8);

    // Body
    R32 diff = 9.0f*scale;

    // Offset beginning position by diff*(n_backbones-1)/2 such that the agent is centered.
    pos -= V3((n_backbones-1)*diff/2.0f, 0, 0);

    agent->body.body = CreateArray<int>(arena, n_backbones);
    agent->body.target_positions = CreateArray<Vec3>(arena, n_backbones);
    for(int i = 0; i < n_backbones; i++)
    {
        R32 r1 = phenotype->backbone_radius[i];
        AddJoint(skele, pos, r1, color);
        int idx = (int)skele->particles.size-1;
        agent->body.body.PushBack(idx);
        agent->body.target_positions.PushBack(pos);

        pos += i < n_backbones-1 ? V3(diff, 0, 0) : V3(0,0,0);

        if(idx==0) continue;

        int prev_idx = idx-1;
        R32 r0 = phenotype->backbone_radius[prev_idx];
        Connect(skele, prev_idx, r0*2, i, r1*2, color);
    }

    for(int i = 0; i < n_backbones; i++)
    {
        if(!phenotype->has_leg[i]) continue;
        R32 knee_size = agent->phenotype->knee_size[i];
        R32 foot_size = agent->phenotype->foot_size[i];
        InitLeg(agent, agent->legs.PushBack(), i, -1, knee_size, foot_size, color);
        InitLeg(agent, agent->legs.PushBack(), i, +1, knee_size, foot_size, color);
    }

    InitLeg(agent, &agent->l_arm, n_backbones-1, -1, phenotype->elbow_size, phenotype->hand_size, color);
    InitLeg(agent, &agent->r_arm, n_backbones-1, -1, phenotype->elbow_size, phenotype->hand_size, color);

    // Head
    pos += V3(0,0,diff);
    agent->head.target_position = pos;
    AddJoint(skele, agent->head.target_position, scale*2, color);
    agent->head.idx = (int)skele->joints.size-1;

    // Connect head to last bodypart in body
    int last_idx = agent->body.body.Last();
    Connect(skele, last_idx, scale, agent->head.idx, scale, color);

    // Transform the entire skeleton to the current agent position and reset the speeds.
    Vec3 center = V3(agent->pos.x, agent->pos.y, 0.0f);
    Vec2 forward = V2Polar(agent->orientation, 1.0f);
    for(Verlet3& v : skele->particles)
    {
        v.pos = XForm(center, forward, v.pos);
        v.old_pos = v.pos;
    }
}

bool EditFloatArray(const char* labelPrefix, float* array, int count, float min, float max)
{
    bool changed = false;
    for (int i = 0; i < count; ++i)
    {
        char label[32];
        snprintf(label, sizeof(label), "%s[%d]", labelPrefix, i);
        changed |= ImGui::SliderFloat(label, &array[i], min, max);
    }
    return changed;
}

bool EditBoolArray(const char* labelPrefix, uint32_t* array, int count)
{
    bool changed = false;
    for (int i = 0; i < count; ++i)
    {
        char label[32];
        snprintf(label, sizeof(label), "%s[%d]", labelPrefix, i);
        bool value = array[i];
        if (ImGui::Checkbox(label, &value))
        {
            changed = true;
            array[i] = static_cast<uint32_t>(value);
        }
    }
    return changed;
}

bool EditCreatureWindow(PhenoType* pheno)
{
    bool changed = false;
    // Edit the number of backbones with clamping
    ImGui::Text("Backbones");
    if (ImGui::InputInt("Number of Backbones", &pheno->n_backbones)) 
    {
        pheno->n_backbones = Clamp(1, pheno->n_backbones, 12);
        changed = true;
    }

    // Edit backbone radii
    ImGui::Text("Backbone Radii");
    changed |= EditFloatArray("Radius", pheno->backbone_radius.v, pheno->n_backbones, 0.1f, 6.0f);

    // Edit has_leg flags
    ImGui::Text("Legs");
    changed |= EditBoolArray("Has Leg", pheno->has_leg.data, pheno->n_backbones);

    // Edit knee sizes
    ImGui::Text("Knee Sizes");
    changed |= EditFloatArray("Knee Size", pheno->knee_size.v, pheno->n_backbones, 0.1f, 6.0f);

    // Edit foot sizes
    ImGui::Text("Foot Sizes");
    changed |= EditFloatArray("Foot Size", pheno->foot_size.v, pheno->n_backbones, 0.1f, 6.0f);

    ImGui::Text("Step radius");
    changed |= EditFloatArray("Step radius", pheno->step_radius.v, pheno->n_backbones, 0.1f, 6.0f);

    // Edit elbow size
    ImGui::Text("Limb Sizes");
    changed |= ImGui::SliderFloat("Elbow Size", &pheno->elbow_size, 0.1f, 6.0f);

    // Edit hand size
    changed |= ImGui::SliderFloat("Hand Size", &pheno->hand_size, 0.1f, 6.0f);

    Vec4 color = ColorToVec4(pheno->color);
    changed |= ImGui::ColorPicker3("Color", color.elements);
    pheno->color = Vec4ToColor(color);
    return changed;
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

    ImGui::Begin("Edit Creature");
    bool reset_creature = false;
    if(ImGui::Button("Randomize"))
    {
        reset_creature = true;
        InitRandomPhenotype(agent->phenotype);
    }
    if(EditCreatureWindow(agent->phenotype))
    {
        reset_creature = true;

    }
    ImGui::End();

    if(reset_creature)
    {
        // TODO: FIX THIS MEMORY LEAK
        agent->skeleton = CreateSkeleton(editor->editor_arena, max_joints, max_joints*2);
        InitAgentSkeleton(editor->editor_arena, agent);
    }

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

    R32 grayscale = 0.6f;
    U32 platform_color = Vec4ToColor(grayscale, grayscale, grayscale, 1.0f);
    R32 shade = 0.6f;
    U32 platform_shade = Vec4ToColor(grayscale*shade, grayscale*shade, grayscale*shade, 1.0f);
    R32 platform_r = walk_radius+25.0f;
    R32 platform_height = 35.0f;
    R32 pedestal_r = platform_r*0.77f;
    R32 pedestal_height = 200.0f;
    AtlasRegion* square = renderer->renderer->square;

    RenderZCircle(renderer, V3(0,0,-pedestal_height), pedestal_r, platform_shade);
    RenderYRect(renderer, V3(0,0,-pedestal_height/2.0f), V2(pedestal_r*2, pedestal_height), square, platform_shade);
    RenderZCircle(renderer, V3(0,0,-platform_height), platform_r, platform_shade);
    RenderYRect(renderer, V3(0,0,-platform_height/2.0f), V2(platform_r*2, platform_height), square, platform_shade);
    RenderZCircle(renderer, V3(0,0,0), platform_r, platform_color);

    // Draw mouse position
    Vec3 mouse = TiltedMouseToWorld(&renderer->cam, input, window->width, window->height);
    R32 cursor_line_width = 0.1f;
    R32 cursor_radius = 1.0f+sinf(editor->time*2)*0.25f;
    RenderZLineCircle(renderer, mouse, cursor_radius, cursor_line_width, Color_Cyan);

    Skeleton* skeleton = agent->skeleton;

    UpdateSkeleton(skeleton);
    UpdateAgentSkeleton(agent);
    RenderSkeleton(renderer, skeleton);

    Vec3 direction = V3(0,0,0);
    direction.xy = V2Polar(agent->orientation, 1.0f);
    Vec3 agent_pos = V3(agent->pos.x, agent->pos.y, 0);
    
    // Draw cute face
    Vec3 head_pos = skeleton->joints[agent->head.idx].v->pos;
    R32 eye_r = 1.0f;
    R32 pupil_r = eye_r/3.0f;
    R32 head_r = 2.0f;

    Vec3 eye_left = XForm(head_pos, direction.xy, V3(1,-1,0));
    Vec3 eye_right = XForm(head_pos, direction.xy, V3(1,1,0));

    if(RandomR32Debug(0, 1) < 0.015) skeleton->blink = 8;
    if(skeleton->blink) skeleton->blink--;
    if(!skeleton->blink)
    {
        RenderCircle(renderer, eye_left, eye_r, Color_White);
        RenderCircle(renderer, eye_right, eye_r, Color_White);
        RenderCircle(renderer, eye_left, pupil_r, Color_Black);
        RenderCircle(renderer, eye_right, pupil_r, Color_Black);
    }
    else
    {
        RenderCircle(renderer, eye_left, eye_r, agent->phenotype->color);
        RenderCircle(renderer, eye_right, eye_r, agent->phenotype->color);
    }

    // Debug rendering
    RenderCircle(renderer, agent_pos, 0.1f, Color_Red);
    for(Leg& leg : agent->legs)
    {
        Vec3 leg_pos = XForm(agent_pos, direction.xy, leg.target_offset);
        RenderCircle(renderer, leg_pos, 0.1f, Color_Green);
        RenderZLineCircle(renderer, leg_pos, leg.r, 0.03f, Color_Green);
    }

    // Render entire thing 
    Render(renderer->renderer, cam, window->width, window->height);

    editor->time += 1.0f/60.0f;

    return 0;
}

void
InitEditorScreen(EditorScreen* editor)
{
    // Might be a bit much memory for the editor.
    MemoryArena* arena = editor->editor_arena = CreateMemoryArena(MegaBytes(32));
    editor->time = 0.0f;
    editor->cam.pos = V2(0,0);
    editor->cam.scale = 1;
    editor->renderer = CreateTiltedRenderer(arena);
    editor->renderer->cam.angle = -PI_R32/4.0f;
    editor->renderer->cam.scale = 6.0;
    editor->agent = PushNewStruct(arena, Agent);
    editor->agent->skeleton = CreateSkeleton(arena, max_joints, max_joints*2);
    editor->agent->pos = V2(0, -walk_radius);

    PhenoType* pheno = editor->agent->phenotype = CreatePhenotype(arena, 12);
    InitRandomPhenotype(pheno);
    InitAgentSkeleton(arena, editor->agent);
}
