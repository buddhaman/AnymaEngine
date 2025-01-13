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
    if(ImGui::Button("Mutate"))
    {
        reset_creature = true;
        MutatePhenotype(agent->phenotype);
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

    // Make the agent walk
    Vec3 direction = V3(0,0,0);
    direction.xy = V2Polar(agent->orientation, 1.0f);
    agent->pos += direction.xy * speed;
    Vec3 perp = V3(-direction.y, direction.x, 0);
    agent->orientation += turn_speed;
    UpdateAgentSkeleton(agent);
    RenderAgent(renderer, agent);

    Vec3 agent_pos = V3(agent->pos.x, agent->pos.y, 0);
    
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
