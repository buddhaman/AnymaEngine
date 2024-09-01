#include <functional>
#include <thread>

#include <GL/glew.h>
#include <imgui.h>
#include <implot.h>

#include "SimulationScreen.h"
#include "DebugInfo.h"
#include "Lucide.h"

const char* vertex_shader_source = R"glsl(
    #version 330 core
    layout (location = 0) in vec2 a_position;
    layout (location = 1) in vec2 a_texture;
    layout (location = 2) in vec4 a_color;

    uniform mat3 u_transform;

    out vec4 v_color;

    void main() 
    {
        vec3 pos = u_transform * vec3(a_position, 1.0);
        v_color = a_color;
        gl_Position = vec4(pos.xy, 0.0, 1.0);
    }
)glsl";

const char* fragment_shader_source = R"glsl(
    #version 330 core

    in vec4 v_color;

    out vec4 frag_color;
    
    void main() 
    {
        vec3 gammaCorrectedColor = pow(v_color.rgb, vec3(1.0/2.2));
        frag_color = vec4(gammaCorrectedColor, v_color.a);
    }
)glsl";

static void
DoScreenWorldUpdate(SimulationScreen* screen)
{
    World* world = screen->world;

    UpdateWorldChanges(world);

    #if 0
    SortAgentsIntoMultipleChunks(screen->world);
    //screen->thread_pool->AddJob(new std::function<void()>([screen, world](){
        for(int y = 0; y < world->y_chunks; y++)
        for(int x = 0; x < world->x_chunks; x++)
        {
            ChunkCollisions(world, x, y);
        }
    //}));
    //screen->thread_pool->AddJob(new std::function<void()>([world](){
        UpdateWorld(world);
    //}));
    //screen->thread_pool->WaitForFinish();
    #else
    SortAgentsIntoMultipleChunks(screen->world);
    for(int y = 0; y < world->y_chunks; y++)
    for(int x = 0; x < world->x_chunks; x++)
    {
        ChunkCollisions(world, x, y);
    }

    ThreadPool* thread_pool = screen->thread_pool;
    int num_ranges = thread_pool->workers.size;
    DynamicArray<int> ranges(num_ranges+1);
    int range_size = world->agents.size / num_ranges;
    for(int i = 0; i < num_ranges; i++)
    {
        ranges.PushBack(range_size*i);
    }
    ranges.PushBack(world->agents.size);

    for(int i = 0; i < ranges.size-1; i++)
    {
        thread_pool->AddJob(new std::function<void()>([i, world, &ranges]() {
            UpdateAgentSensorsAndBrains(world, ranges[i], ranges[i+1]);
        }));
    }
    thread_pool->StartAndWaitForFinish();
    
    for(int i = 0; i < ranges.size-1; i++)
    {
        //thread_pool->AddJob(new std::function<void()>([i, world, &ranges](){
        UpdateAgentBehavior(world, ranges[i], ranges[i+1]);
        //}));
    }
    //thread_pool->WaitForFinish();


    #endif
}

static void
DrawGrid(Mesh2D* mesh, Vec2 cam_min, Vec2 cam_max, Vec2 world_min, Vec2 world_max, R32 grid_size, R32 thickness, U32 color)
{
    Vec2 draw_min = V2(Max(cam_min.x, world_min.x), Max(cam_min.y, world_min.y));
    Vec2 draw_max = V2(Min(cam_max.x, world_max.x), Min(cam_max.y, world_max.y));

    I32 x_chunk_start = (I32)floor(draw_min.x / grid_size);
    I32 x_chunk_end = (I32)ceil(draw_max.x / grid_size);
    I32 y_chunk_start = (I32)floor(draw_min.y / grid_size);
    I32 y_chunk_end = (I32)ceil(draw_max.y / grid_size);

    Vec2 u = V2(0, 0);

    // Draw vertical lines
    for (I32 x_chunk = x_chunk_start; x_chunk <= x_chunk_end; x_chunk++)
    {
        R32 x = x_chunk * grid_size;
        PushRect(mesh, V2(x - thickness / 2.0f, draw_min.y), V2(thickness, draw_max.y - draw_min.y), u, u, color);
    }

    // Draw horizontal lines
    for (I32 y_chunk = y_chunk_start; y_chunk <= y_chunk_end; y_chunk++)
    {
        R32 y = y_chunk * grid_size;
        PushRect(mesh, V2(draw_min.x, y - thickness / 2.0f), V2(draw_max.x - draw_min.x, thickness), u, u, color);
    }
}

static void
DoScreenWorldRender(SimulationScreen* screen, Window* window)
{
    Shader* shader = &screen->shader;
    Mesh2D* mesh = &screen->dynamic_mesh;
    World* world = screen->world;
    Camera2D* cam = &screen->cam;

    glUseProgram(shader->program_handle);
    UpdateCamera(cam, window->width, window->height);
    glUniformMatrix3fv(screen->transform_loc, 1, GL_FALSE, &screen->cam.transform.m[0][0]);

    R32 thickness = 0.2f;
    R32 min_scale = 4.0f;
    I32 subdivs = 5;
    if(cam->scale > min_scale)
    {
        R32 factor = log2f(cam->scale) - log2f(min_scale);
        R32 alpha = Clamp(0.0f, factor, 1.0f);
        U32 color = Vec4ToHex(0.3f, 0.3f, 0.3f, alpha);
        DrawGrid(mesh, cam->pos - cam->size/2.0f, cam->pos + cam->size/2.0f, V2(0,0), world->size, world->chunk_size/subdivs, thickness/3.0f, color);
    }

    if(cam->scale > 1.0f)
    {
        R32 alpha = Clamp(0.0f, cam->scale-1.0f, 1.0f);
        U32 color = Vec4ToHex(0.4f, 0.4f, 0.4f, alpha);
        DrawGrid(mesh, cam->pos - cam->size/2.0f, cam->pos + cam->size/2.0f, V2(0,0), world->size, world->chunk_size, thickness, color);
    }

    if(screen->show_chunk_occupancy)
    {
        DrawChunks(world, mesh, &screen->cam);
    }

    // Only difference between these if statements is the color but thats done on purpose.
    if(screen->hovered_agent)
    {
        R32 time_factor = 0.25f*sinf(screen->time*10);
        R32 radius = screen->hovered_agent->radius + screen->extra_selection_radius + time_factor;
        R32 line_width = 0.4f;
        U32 color = 0xff88aaff;
        PushLineNGon(mesh, screen->hovered_agent->pos, radius, radius + line_width, 8, screen->time*3, V2(0,0), color);
    }

    if(screen->selected_agent)
    {
        R32 time_factor = 0.25f*sinf(screen->time*10);
        R32 radius = screen->selected_agent->radius + screen->extra_selection_radius + time_factor;
        R32 line_width = 0.4f;
        U32 color = 0xffaa77ff;
        PushLineNGon(mesh, screen->selected_agent->pos, radius, radius + line_width, 8, screen->time*3, V2(0,0), color);
        RenderEyeRays(mesh, screen->selected_agent);
        std::cout << "I just got selected " <<  screen->selected_agent << std::endl;
    }

    RenderWorld(world, mesh, &screen->cam, screen->overlay);

    BufferData(mesh, GL_DYNAMIC_DRAW);
    Draw(mesh);
    Clear(mesh);
}

static inline bool
ImGuiInputFloat(const char* text, R32* value, R32 min, R32 max)
{
    if(ImGui::InputFloat(text, value))
    {
        *value = Clamp(min, *value, max);
        return true;
    }
    return false;
}

static inline bool
ImGuiInputInt(const char* text, int* value, int min, int max)
{
    if(ImGui::InputInt(text, value))
    {
        *value = Clamp(min, *value, max);
        return true;
    }
    return false;
}

void
EditSettings(SimulationScreen* screen)
{
    ImGui::SeparatorText("World settings");
    SimulationSettings* settings = &global_settings;
    World* world = screen->world;
    bool changed = false;
    changed |= ImGuiInputInt("Max agents", &settings->max_agents, 1, 256000);
    changed |= ImGuiInputInt("Initial agents", &settings->n_initial_agents, 1, settings->max_agents);
    changed |= ImGuiInputFloat("Chunk size", &settings->chunk_size, 4.0f, 400.0f);
    changed |= ImGuiInputInt("X chunks", &settings->x_chunks, 1, 256);
    changed |= ImGuiInputInt("Y chunks", &settings->y_chunks, 1, 256);
    changed |= ImGuiInputFloat("Mutation rate", &world->mutation_rate, 0.0f, 1.0f);
    changed |= ImGuiInputInt("Energy on hit", &settings->energy_transfer_on_hit, 1, 10);
    (void)changed; // Not used yet/anymore.

    if(ImGui::Button("Add herbivore"))
    {
        Vec2 pos = screen->cam.pos;
        if(InBounds({V2(0,0), world->size}, pos) && CanAddAgent(world, pos))
        {
            AddAgent(world, AgentType_Herbivore, pos);
        }
    }

    if(ImGui::Button("Add carnivore"))
    {
        Vec2 pos = screen->cam.pos;
        if(InBounds({V2(0,0), world->size}, pos) && CanAddAgent(world, pos))
        {
            AddAgent(world, AgentType_Carnivore, pos);
        }
    }

    if(ImGui::Button("Restart"))
    {
        RestartWorld(screen);
    }

    ImGui::SeparatorText("Visual");

    // ComboBox for selecting ColorOverlay
    const char* overlayItems[] = { "Agent Type", "Agent Genes" };
    int currentItem = static_cast<int>(screen->overlay);

    if (ImGui::Combo("Color Overlay", &currentItem, overlayItems, IM_ARRAYSIZE(overlayItems)))
    {
        screen->overlay = static_cast<ColorOverlay>(currentItem);
    }
    ImGui::Checkbox("Show chunk occupancy", &screen->show_chunk_occupancy);
}

static void
DoDebugInfo(SimulationScreen* screen, Window* window)
{
    World* world = screen->world;

    ImGui::Text("FPS: %.0f", window->fps);
    ImGui::Text("Update: %.2f millis", window->update_millis);
    ImGui::Text("Camera scale: %.2f", screen->cam.scale);
    ImGui::Text("Number of cores: %lld", screen->thread_pool->workers.size);
    ImGuiMemoryArena(world->arena, "Static memory");
    ImGuiMemoryArena(world->lifespan_arena, "Current lifespan arena");
    ImGuiMemoryArena(world->lifespan_arena_old, "Old lifespan arena");
    ImGui::Text("Number of agents: %zu, (C: %d, H: %d)", 
            world->agents.size, world->num_agenttype[AgentType_Carnivore], world->num_agenttype[AgentType_Herbivore]);
}

void
DoAgentInfo(SimulationScreen* screen, Agent* agent)
{
    World* world = screen->world;
    ChunkCoordinates coords = GetChunkCoordinatesFromWorldPos(screen->world, agent->pos);
    ImGui::Text("Chunk: %u %u", coords.x, coords.y);

    ImGui::Text("%d ticks until reproduction.", agent->ticks_until_reproduce);
    R32 reproduction_factor = (R32)agent->ticks_until_reproduce/(R32)world->max_lifespan;
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, IM_COL32(0, 255, 0, 255));
    ImGui::ProgressBar(reproduction_factor);
    ImGui::PopStyleColor();

    ImGui::Text("%d ticks until out of energy.", agent->energy);
    R32 energy_fraction = (R32)agent->energy/(R32)world->max_lifespan;
    ImGui::ProgressBar(energy_fraction);
}

void 
DoControlWindow(SimulationScreen* screen)
{
    if(ImGui::Button(screen->isPaused ? ICON_LC_PLAY : ICON_LC_PAUSE))
    {
        screen->isPaused = !screen->isPaused;
    }
    ImGui::SameLine();
    I64 seconds = screen->world->ticks/60;
    I64 minutes = seconds/60;
    I64 hours = minutes/60;
    I64 days = minutes/60;
    ImGui::Text("At tick: %lld", screen->world->ticks);
    ImGui::Text(ICON_LC_CLOCK_11 " %lld days, %02lld:%02lld:%02lld", days, hours%24, minutes%60, seconds%60);
    ImGui::SliderInt("Speed " ICON_LC_RABBIT, &screen->updates_per_frame, 1, 25);
}

void
DoStatisticsWindow(SimulationScreen* screen)
{
    World* world = screen->world;

    ImPlotFlags updatetime_plot_flags = ImPlotFlags_NoBoxSelect | 
                        ImPlotFlags_NoInputs | 
                        ImPlotFlags_NoFrame | 
                        ImPlotFlags_NoLegend;

    ImPlot::SetNextAxesLimits(0, screen->update_times.size, 0, 80.0f, 0);
    if(ImPlot::BeginPlot("Frame update time", V2(-1, 200), updatetime_plot_flags))
    { 
        ImPlot::PlotBars("Update time", screen->update_times.data, screen->update_times.size, 1);
        ImPlot::EndPlot();
    }

    // Collect population info
    if((screen->track_population_per-=screen->updates_per_frame) <= 0 && !screen->isPaused)
    {
        screen->track_population_per = 30;
        screen->num_herbivores.Shift(-1);
        screen->num_herbivores[screen->num_herbivores.size-1] = world->num_agenttype[AgentType_Herbivore];
        screen->num_carnivores.Shift(-1);
        screen->num_carnivores[screen->num_carnivores.size-1] = world->num_agenttype[AgentType_Carnivore];
    }

    DynamicArray<R32> x_axis(screen->num_herbivores.size);
    x_axis.Fill();
    x_axis.Apply([](int i, R32& val) {val = i;});

    DynamicArray<R32> bottom(screen->num_herbivores.size);
    bottom.FillAndSetValue(0);

    I32 max_agents = global_settings.max_agents;
    ImPlot::SetNextAxesLimits(0, screen->num_herbivores.size, 0, max_agents, ImPlotCond_Always);
    if(ImPlot::BeginPlot("Population", V2(-1, 300), updatetime_plot_flags))
    {
        ImPlot::PushStyleColor(ImPlotCol_Fill, V4(1.0f, 0.3f, 0.3f, 0.6f));
        ImPlot::PlotShaded("Carnivores", x_axis.data, bottom.data, screen->num_carnivores.data, screen->num_carnivores.size);
        ImPlot::PopStyleColor();

        bottom.Apply([&screen](int i, R32& x){ x = screen->num_carnivores[i] + screen->num_herbivores[i]; });

        ImPlot::PushStyleColor(ImPlotCol_Fill, V4(0.3f, 1.0f, 0.3f, 0.6f));
        ImPlot::PlotShaded("Herbivores", x_axis.data, screen->num_carnivores.data, bottom.data, screen->num_herbivores.size);
        ImPlot::PopStyleColor();

        ImPlot::EndPlot();
    }

    ImGuiChunkDistribution(screen->world);
    if(screen->selected_agent)
    {
        ImGuiBrainVisualizer(screen->selected_agent->brain);
    }
}

void
UpdateSimulationScreen(SimulationScreen* screen, Window* window)
{
    InputHandler* input = &window->input;
    World* world = screen->world;
    Camera2D* cam = &screen->cam;

    screen->update_times.Shift(-1);
    screen->update_times[screen->update_times.size-1] = window->update_millis;

    // TODO: Do not hardcode this. 1.0f/60.0f;
    screen->time +=  1.0f/60.0f;

    ImGui::BeginMainMenuBar();
    if(ImGui::BeginMenu("Window"))
    {
        if(ImGui::MenuItem("Show debug window" ICON_LC_ALBUM, nullptr, &screen->show_debug_window))
        {
        }
        if(ImGui::MenuItem("Show edit window", nullptr, &screen->show_edit_window))
        {
        }
        if(ImGui::MenuItem("Show statistics window", nullptr, &screen->show_statistic_window))
        {
        }
        if(ImGui::MenuItem("Show control window", nullptr, &screen->show_statistic_window))
        {
        }
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();

    if(screen->show_debug_window)
    {
        ImGui::Begin(ICON_LC_BUG " Debug and Agents");
        DoDebugInfo(screen, window);
        if(screen->selected_agent)
        {
            ImGui::SeparatorText("Selected agent");
            DoAgentInfo(screen, screen->selected_agent);
        }
        ImGui::End();
    }

    if(screen->show_edit_window)
    {
        ImGui::Begin(ICON_LC_DNA " Edit");
        EditSettings(screen);
        ImGui::End();
    }

    if(screen->show_statistic_window)
    {
        ImGui::Begin(ICON_LC_BAR_CHART_BIG " Statistics");
        DoStatisticsWindow(screen);
        ImGui::End();
    }

    if(screen->show_control_window)
    {
        ImGui::Begin(ICON_LC_GAUGE " Control simulation");
        DoControlWindow(screen);
        ImGui::End();
    }

    Vec2 world_mouse_pos = MouseToWorld(&screen->cam, input, window->width, window->height);
    ImGuiIO& io = ImGui::GetIO();
    screen->hovered_agent = nullptr;
    if(!io.WantCaptureMouse)
    {
        if(IsKeyDown(input, InputAction_W))
        {
            cam->pos.y += .1f/cam->scale;
        }

        if(input->mousedown[0])
        {
            Vec2 diff = input->mouse_delta / (cam->scale);
            cam->pos.x -= diff.x;
            cam->pos.y += diff.y;

        }

        screen->hovered_agent = SelectFromWorld(world, world_mouse_pos, screen->extra_selection_radius);

        if(IsMouseClicked(input, 0))
        {
            screen->selected_agent = screen->hovered_agent;
        }

        cam->scale = cam->scale *= powf(1.05f, input->mouse_scroll);
    }

    if(!screen->isPaused)
    {
        for(int i = 0; i < screen->updates_per_frame; i++)
        {
            DoScreenWorldUpdate(screen);
        }
    }

    DoScreenWorldRender(screen, window);
}

void
RestartWorld(SimulationScreen* screen)
{
    ClearArena(screen->world_arena);
    screen->world = CreateWorld(screen->world_arena);
}

void
InitSimulationScreen(SimulationScreen* screen)
{
    screen->world_arena = CreateMemoryArena(MegaBytes(256));

    RestartWorld(screen);
    screen->cam.pos = screen->world->size/2.0f;

    screen->shader = CreateShader(vertex_shader_source, fragment_shader_source);
    screen->transform_loc = glGetUniformLocation(screen->shader.program_handle, "u_transform");

    screen->update_times.FillAndSetValue(0);
    screen->num_carnivores.FillAndSetValue(0);
    screen->num_herbivores.FillAndSetValue(0);

    I32 num_cores = std::thread::hardware_concurrency();
    std::cout << "Num cores = " << num_cores << std::endl;

    U32 num_threads = Max(num_cores-1, 1);
    screen->thread_pool = CreateThreadPool(num_threads);
}
