#include <GL/glew.h>
#include <imgui.h>
#include <implot.h>

#include "SimulationScreen.h"
#include "DebugInfo.h"

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
        frag_color = v_color;
    }
)glsl";

static void
DoScreenWorldUpdate(SimulationScreen* screen)
{
    World* world = screen->world;
    //SortAgentsIntoChunks(&world);
    SortAgentsIntoMultipleChunks(screen->world);
    for(int y = 0; y < world->y_chunks; y++)
    for(int x = 0; x < world->x_chunks; x++)
    {
        ChunkCollisions(world, x, y);
    }
    UpdateWorld(world);
}

static void
DoScreenWorldRender(SimulationScreen* screen, Window* window)
{
    Shader* shader = &screen->shader;
    Mesh2D* mesh = &screen->dynamic_mesh;
    Agent* selected = screen->selected;
    World* world = screen->world;

    glUseProgram(shader->program_handle);
    UpdateCamera(&screen->cam, window->width, window->height);

    glUniformMatrix3fv(screen->transform_loc, 1, GL_FALSE, &screen->cam.transform.m[0][0]);
    if(selected)
    {
        PushRect(mesh, selected->pos+V2(1,1), V2(1,1), V2(0,0), V2(0,0), 0xffaa77ff);
        RenderEyeRays(mesh, selected);
    }

    if(screen->show_chunks)
    {
        RenderChunks(world, mesh, &screen->cam);
    }
    RenderWorld(world, mesh, &screen->cam);

    BufferData(mesh, GL_DYNAMIC_DRAW);
    Draw(mesh);
    Clear(mesh);
}

void
EditSettings(SimulationScreen* screen)
{
    ImGui::SeparatorText("World settings");
    bool changed = false;
    changed |= ImGui::InputInt("Max agents", &screen->settings.max_agents);
    changed |= ImGui::InputInt("Initial agents", &screen->settings.n_initial_agents);
    changed |= ImGui::InputInt("Herbivore reproduction ticks", &screen->world->herbivore_reproduction_ticks);
    changed |= ImGui::InputInt("Carnivore reproduction ticks", &screen->world->carnivore_reproduction_ticks);
    changed |= ImGui::InputFloat("Chunk size", &screen->settings.chunk_size);
    changed |= ImGui::InputInt("X chunks", &screen->settings.x_chunks);
    changed |= ImGui::InputInt("Y chunks", &screen->settings.y_chunks);

    if(changed)
    {
        // Validate
        screen->settings.max_agents = Clamp(1, screen->settings.max_agents, 256000);
        screen->settings.n_initial_agents = Clamp(1, screen->settings.n_initial_agents, screen->settings.max_agents);
        screen->settings.chunk_size = Clamp(1.0f, screen->settings.chunk_size, 10000000.0f);
        screen->settings.x_chunks = Clamp(1, screen->settings.x_chunks, 10000);
        screen->settings.y_chunks = Clamp(1, screen->settings.y_chunks, 10000);
    }

    if(ImGui::Button("Restart"))
    {
        RestartWorld(screen);
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

    ImGui::BeginMainMenuBar();
    if(ImGui::BeginMenu("Window"))
    {
        if(ImGui::MenuItem("Show performance stats", nullptr, &screen->show_debug_window))
        {
        }
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();

    if(screen->show_debug_window)
    {
        ImGui::Begin("Performance stats and settings");

        ImGui::Text("FPS: %.0f", window->fps);
        ImGui::Text("Update: %.2f millis", window->update_millis);
        ImGui::Text("Camera scale: %.2f", screen->cam.scale);
        ImGui::Text("Static memory used in world: %zu/%zuMB", screen->world->arena->used/(1024U*1024U), screen->world->arena->size/(1024U*1024U));
        ImGui::Text("Number of agents: %zu", screen->world->agents.size);
        ImGui::Text("At tick: %lu", screen->world->ticks);

        if(screen->selected)
        {
            ImGui::SeparatorText("Selected agent");
            ChunkCoordinates coords = GetChunkCoordinatesFromWorldPos(screen->world, screen->selected->pos);
            ImGui::Text("Chunk: %u %u", coords.x, coords.y);
            ImGui::Text("%d ticks until reproduction.", screen->selected->ticks_until_reproduce);
            ImGui::Text("%d ticks until out of energy.", screen->selected->energy);
        }

        EditSettings(screen);

        ImGui::Checkbox("Show chunks", &screen->show_chunks);

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

        ImGuiChunkDistribution(screen->world);

        ImGui::End();
    }

    Vec2 world_mouse_pos = MouseToWorld(&screen->cam, input, window->width, window->height);
    ImGuiIO& io = ImGui::GetIO();
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

            Agent* found = SelectFromWorld(world, world_mouse_pos);
            screen->selected = found;
        }

        cam->scale = cam->scale *= powf(1.05f, input->mouse_scroll);
    }

    DoScreenWorldUpdate(screen);
    DoScreenWorldRender(screen, window);
}

void
RestartWorld(SimulationScreen* screen)
{
    screen->world = CreateWorld(screen->world_arena, &screen->settings);
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
}
