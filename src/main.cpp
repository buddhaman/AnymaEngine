#include <SDL.h>
#include <GL/glew.h>
#include <imgui.h>

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

int main(int argc, char** argv) 
{
     // Initialize random seed once. This will be removed by my own rng.
    srand((time(nullptr)));

    Window* window = CreateWindow(1280, 720);
    if(!window)
    {
        return -1;
    }

    Shader shader = CreateShader(vertex_shader_source, fragment_shader_source);

    // Find uniforms
    U32 transform_loc = glGetUniformLocation(shader.program_handle, "u_transform");

    // Camera
    Camera2D cam;
    cam.scale = 50.0f;

    Mesh2D mesh;
    Vec2 uv = V2(0,0);
    // Main loop
    bool running = true;
    SDL_Event event;

    World world;
    InitWorld(&world);

    cam.pos = world.size/2.0f;

    InputHandler* input = &window->input;
    while (window->running) 
    {
        WindowBegin(window);
        ImGui::Begin("Hellow i set it  up agian");
        ImGui::Text("I just setup imgui");
        ImGui::Text("FPS: %d", window->fps);
        ImGui::Text("update: %.2f millis", window->update_millis);
        ImGui::Text("carry: %.2f", window->carry_millis);
        ImGui::End();

        // Draw the triangle
        glUseProgram(shader.program_handle);

        static float time = 0.0f;
        time += 0.016f;

        //cam.pos = V2(sinf(time), 0);
        UpdateCamera(&cam, window->width, window->height);

        glUniformMatrix3fv(transform_loc, 1, GL_FALSE, &cam.transform.m[0][0]);

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

        Vec2 pos = V2(-0.5, -0.5);
        PushRect(&mesh, pos, V2(1,1), V2(0,0), V2(0,0), 0xffaa77ff);

        UpdateWorld(&world);
        RenderWorld(&world, &mesh);
        
        BufferData(&mesh, GL_DYNAMIC_DRAW);
        Draw(&mesh);
        Clear(&mesh);

        WindowEnd(window);
    }
    
    DestroyWindow(window);

    return 0;
}
