#include <SDL.h>
#include <GL/glew.h>
#include <imgui.h>
#include <implot.h>

#include "AnymUtil.h"
#include "Array.h"
#include "TimVectorMath.h"

#include "Mesh2D.h"
#include "Camera2D.h"
#include "Window.h"
#include "InputHandler.h"
#include "Shader.h"

#include "World.h"

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

void
PrintArray(Array<int>& arr)
{
    for(int i = 0; i < arr.size; i++)
    {
        std::cout << arr[i] << ", ";
    }
    std::cout << std::endl;
}

int main(int argc, char** argv) 
{
     // Initialize random seed once. This will be replaced by my own rng.
    srand((time(nullptr)));

    Window* window = CreateWindow(1280, 720);
    if(!window)
    {
        return -1;
    }

    Shader shader = CreateShader(vertex_shader_source, fragment_shader_source);
    U32 transform_loc = glGetUniformLocation(shader.program_handle, "u_transform");

    // Camera
    Camera2D cam;
    cam.scale = 50.0f;

    Mesh2D mesh;
    Vec2 uv = V2(0,0);

    World world;
    InitWorld(&world);

    cam.pos = world.size/2.0f;

    DynamicArray<R32> update_times(120);
    update_times.FillAndSetValue(0);

    window->fps = 60;

    InputHandler* input = &window->input;

    // Main loop
    bool running = true;
    SDL_Event event;
    while (window->running) 
    {
        WindowBegin(window);
        ImGui::Begin("Hellow i set it  up agian");
        ImGui::Text("I just setup imgui");
        ImGui::Text("FPS: %.0f", window->fps);
        ImGui::Text("Update: %.2f millis", window->update_millis);
        ImGui::Text("Carry: %.2f", window->carry_millis);
        ImGui::Text("Camera scale: %.2f", cam.scale);
        ImGui::Separator();
        ImGui::Text("Number of agents: %zu", world.agents.size);

        update_times.Shift(-1);
        update_times[update_times.size-1] = window->update_millis;

        ImPlotFlags updatetime_plot_flags = ImPlotFlags_NoBoxSelect | 
                            ImPlotFlags_NoInputs | 
                            ImPlotFlags_NoFrame | 
                            ImPlotFlags_NoLegend;

        ImPlot::SetNextAxesLimits(0, update_times.size, 0, 32.0f, 0);
        if(ImPlot::BeginPlot("Frame update time", V2(-1, 200), updatetime_plot_flags))
        {
            ImPlot::PlotBars("Update time", update_times.data, update_times.size, 1);
            ImPlot::EndPlot();
        }

        ImGui::End();

        // Draw the triangle
        glUseProgram(shader.program_handle);

        static float time = 0.0f;
        time += 0.016f;

        //cam.pos = V2(sinf(time), 0);
        UpdateCamera(&cam, window->width, window->height);

        glUniformMatrix3fv(transform_loc, 1, GL_FALSE, &cam.transform.m[0][0]);

        ImGuiIO& io = ImGui::GetIO();
        if(!io.WantCaptureMouse)
        {
            if(IsKeyDown(input, InputAction_W))
            {
                cam.pos.y += .1f/cam.scale;
            }

            if(input->mousedown[0])
            {
                Vec2 diff = input->mouse_delta / (cam.scale);
                cam.pos.x -= diff.x;
                cam.pos.y += diff.y;
            }

            cam.scale = cam.scale *= powf(1.05f, input->mouse_scroll);
        }

        Vec2 pos = V2(-0.5, -0.5);
        PushRect(&mesh, pos, V2(1,1), V2(0,0), V2(0,0), 0xffaa77ff);

        UpdateWorld(&world);
        RenderWorld(&world, &mesh, &cam);

        PushLine(&mesh, V2(sinf(time)*2,0), V2(10, sinf(time*2)*3), 0.4f, pos, pos, 0xffaa77ff);
        PushNGon(&mesh, V2(12, 12), 3, 5, time, pos, 0x77ffaaff);
        
        BufferData(&mesh, GL_DYNAMIC_DRAW);
        Draw(&mesh);
        Clear(&mesh);

        WindowEnd(window);
    }
    
    DestroyWindow(window);

    return 0;
}
