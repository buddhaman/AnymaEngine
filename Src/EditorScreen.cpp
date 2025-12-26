#include <imgui.h>
#include <implot.h>
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

// Hidden function - not called
bool EditCreatureWindowHidden(PhenoType* pheno)
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

void EditSoupWindow(EditorScreen* editor)
{
    if(!editor->soup) return;
    
    Soup* soup = editor->soup;
    MemoryArena* arena = editor->editor_arena;
    
    ImGui::Begin("Soup Editor");
    
    // Create new Soup
    static int new_num_neurons = 1000;
    static int new_num_connections = 10;
    ImGui::Text("Create New Soup");
    ImGui::InputInt("Number of Neurons", &new_num_neurons);
    ImGui::InputInt("Connections per Neuron", &new_num_connections);
    new_num_neurons = Clamp(10, new_num_neurons, 10000);
    new_num_connections = Clamp(1, new_num_connections, 1000);
    
    if(ImGui::Button("Create New Soup"))
    {
        editor->soup = CreateSoup(arena, new_num_neurons, new_num_connections);
        soup = editor->soup;
        // Reset history
        editor->active_neuron_history.Clear();
        editor->avg_threshold_history.Clear();
        editor->firing_rate_history.Clear();
    }
    
    ImGui::Separator();
    
    // Edit Soup parameters
    ImGui::Text("Soup Parameters");
    ImGui::SliderFloat("Target Rate", &soup->target_rate, 0.0f, 1.0f);
    ImGui::SliderFloat("Threshold Learning Rate", &soup->eta_threshold, 0.0f, 0.1f);
    ImGui::SliderFloat("Weight Learning Rate", &soup->eta_weight, 0.0f, 0.01f);
    ImGui::SliderFloat("Eligibility Decay", &soup->eligibility_decay, 0.0f, 1.0f);
    
    ImGui::Separator();
    
    // Run controls
    ImGui::Text("Run Controls");
    
    // Play/Pause button
    if(editor->soup_playing)
    {
        if(ImGui::Button(ICON_LC_PAUSE " Pause"))
        {
            editor->soup_playing = false;
        }
    }
    else
    {
        if(ImGui::Button(ICON_LC_PLAY " Play"))
        {
            editor->soup_playing = true;
        }
    }
    
    ImGui::SameLine();
    
    // Step button
    if(ImGui::Button(ICON_LC_STEP_FORWARD " Step"))
    {
        UpdateSoup(*soup);
    }
    
    // Speed control (steps per frame)
    ImGui::SliderInt("Steps Per Frame", &editor->soup_steps_per_frame, 1, 100);
    
    // Execute steps if playing
    if(editor->soup_playing)
    {
        for(int i = 0; i < editor->soup_steps_per_frame; i++)
        {
            UpdateSoup(*soup);
        }
    }
    
    ImGui::Separator();
    
    // Statistics
    int active_count = CountActiveNeurons(*soup);
    R32 firing_rate = soup->neurons.size > 0 ? (R32)active_count / (R32)soup->neurons.size : 0.0f;
    
    R32 avg_threshold = 0.0f;
    for(Neuron& neuron : soup->neurons)
    {
        avg_threshold += neuron.threshold;
    }
    if(soup->neurons.size > 0)
    {
        avg_threshold /= (R32)soup->neurons.size;
    }
    
    ImGui::Text("Statistics");
    ImGui::Text("Total Neurons: %lld", soup->neurons.size);
    ImGui::Text("Active Neurons: %d", active_count);
    ImGui::Text("Firing Rate: %.3f", firing_rate);
    ImGui::Text("Average Threshold: %.3f", avg_threshold);
    
    // Track history
    editor->history_track_counter--;
    if(editor->history_track_counter <= 0)
    {
        editor->history_track_counter = editor->track_history_per;
        
        // Shift arrays and add new data
        const int max_history = 200;
        if(editor->active_neuron_history.size >= max_history)
        {
            editor->active_neuron_history.Shift(-1);
            editor->avg_threshold_history.Shift(-1);
            editor->firing_rate_history.Shift(-1);
        }
        else
        {
            editor->active_neuron_history.PushBack((R32)active_count);
            editor->avg_threshold_history.PushBack(avg_threshold);
            editor->firing_rate_history.PushBack(firing_rate);
        }
        
        // Update last values
        if(editor->active_neuron_history.size > 0)
        {
            editor->active_neuron_history[editor->active_neuron_history.size - 1] = (R32)active_count;
            editor->avg_threshold_history[editor->avg_threshold_history.size - 1] = avg_threshold;
            editor->firing_rate_history[editor->firing_rate_history.size - 1] = firing_rate;
        }
    }
    else if(editor->active_neuron_history.size > 0)
    {
        // Update last value even if not tracking new point
        editor->active_neuron_history[editor->active_neuron_history.size - 1] = (R32)active_count;
        editor->avg_threshold_history[editor->avg_threshold_history.size - 1] = avg_threshold;
        editor->firing_rate_history[editor->firing_rate_history.size - 1] = firing_rate;
    }
    
    ImGui::Separator();
    
    // Visualizations
    ImPlotFlags plot_flags = ImPlotFlags_NoBoxSelect | 
                            ImPlotFlags_NoInputs | 
                            ImPlotFlags_NoFrame | 
                            ImPlotFlags_NoLegend;
    
    // Active neurons over time
    if(editor->active_neuron_history.size > 0)
    {
        DynamicArray<R32> x_axis(editor->active_neuron_history.size);
        x_axis.Fill();
        x_axis.ApplyIndexed([](int i, R32& val) {val = (R32)i;});
        
        ImPlot::SetNextAxesLimits(0, (int)editor->active_neuron_history.size, 0, (R32)soup->neurons.size, ImPlotCond_Always);
        Vec2 plot_size = V2(-1, 150);
        if(ImPlot::BeginPlot("Active Neurons Over Time", ImVec2(plot_size.x, plot_size.y), plot_flags))
        {
            ImPlot::PlotLine("Active", x_axis.data, editor->active_neuron_history.data, (int)editor->active_neuron_history.size);
            ImPlot::EndPlot();
        }
    }
    
    // Firing rate over time
    if(editor->firing_rate_history.size > 0)
    {
        DynamicArray<R32> x_axis(editor->firing_rate_history.size);
        x_axis.Fill();
        x_axis.ApplyIndexed([](int i, R32& val) {val = (R32)i;});
        
        ImPlot::SetNextAxesLimits(0, (int)editor->firing_rate_history.size, 0.0f, 1.0f, ImPlotCond_Always);
        Vec2 plot_size2 = V2(-1, 150);
        if(ImPlot::BeginPlot("Firing Rate Over Time", ImVec2(plot_size2.x, plot_size2.y), plot_flags))
        {
            Vec4 color1 = V4(0.3f, 1.0f, 0.3f, 1.0f);
            ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(color1.x, color1.y, color1.z, color1.w));
            ImPlot::PlotLine("Firing Rate", x_axis.data, editor->firing_rate_history.data, (int)editor->firing_rate_history.size);
            ImPlot::PopStyleColor();
            
            // Draw target rate line
            R32 target_line[2] = {soup->target_rate, soup->target_rate};
            R32 target_x[2] = {0.0f, (R32)editor->firing_rate_history.size};
            Vec4 color2 = V4(1.0f, 0.3f, 0.3f, 0.5f);
            ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(color2.x, color2.y, color2.z, color2.w));
            ImPlot::PlotLine("Target", target_x, target_line, 2);
            ImPlot::PopStyleColor();
            
            ImPlot::EndPlot();
        }
    }
    
    // Average threshold over time
    if(editor->avg_threshold_history.size > 0)
    {
        DynamicArray<R32> x_axis(editor->avg_threshold_history.size);
        x_axis.Fill();
        x_axis.ApplyIndexed([](int i, R32& val) {val = (R32)i;});
        
        ImPlot::SetNextAxesToFit();
        Vec2 plot_size3 = V2(-1, 150);
        if(ImPlot::BeginPlot("Average Threshold Over Time", ImVec2(plot_size3.x, plot_size3.y), plot_flags))
        {
            Vec4 color3 = V4(0.3f, 0.3f, 1.0f, 1.0f);
            ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(color3.x, color3.y, color3.z, color3.w));
            ImPlot::PlotLine("Threshold", x_axis.data, editor->avg_threshold_history.data, (int)editor->avg_threshold_history.size);
            ImPlot::PopStyleColor();
            ImPlot::EndPlot();
        }
    }
    
    // Neuron states visualization (sample of neurons)
    const int sample_size = Min(100, (int)soup->neurons.size);
    if(sample_size > 0)
    {
        DynamicArray<R32> neuron_states(sample_size);
        DynamicArray<R32> neuron_indices(sample_size);
        
        for(int i = 0; i < sample_size; i++)
        {
            int idx = (int)((I64)i * soup->neurons.size / sample_size);
            neuron_states.PushBack((R32)soup->neurons[idx].state);
            neuron_indices.PushBack((R32)i);
        }
        
        ImPlot::SetNextAxesLimits(-1, sample_size, -0.5f, 1.5f, ImPlotCond_Always);
        Vec2 plot_size4 = V2(-1, 150);
        if(ImPlot::BeginPlot("Neuron States (Sample)", ImVec2(plot_size4.x, plot_size4.y), plot_flags))
        {
            Vec4 color4 = V4(0.3f, 0.8f, 0.3f, 0.6f);
            ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4(color4.x, color4.y, color4.z, color4.w));
            ImPlot::PlotBars("States", neuron_indices.data, neuron_states.data, sample_size, 0.5f);
            ImPlot::PopStyleColor();
            ImPlot::EndPlot();
        }
    }
    
    ImGui::End();
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

    // EditCreatureWindowHidden is not called - moved to separate function
    
    // Soup editor window
    EditSoupWindow(editor);

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

    // Make the agent walk
    Vec3 direction = V3(0,0,0);
    direction.xy = V2Polar(agent->orientation, 1.0f);
    agent->pos += direction.xy * speed;
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
    MemoryArena* arena = editor->editor_arena = CreateMemoryArena(MegaBytes(256));
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

    editor->soup = CreateSoup(arena, 1000, 10);
    
    // Tracking arrays are initialized via constructor
    editor->track_history_per = 1;
    editor->history_track_counter = 0;
}
