#include <SDL.h>
#include <GL/glew.h>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

#include "AnymUtil.h"
#include "Array.h"
#include "TimVectorMath.h"

#include "Render2D.h"
#include "Camera2D.h"
#include "Window.h"
#include "InputHandler.h"
#include "Shader.h"

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

void GLAPIENTRY MessageCallback(GLenum source,
                                GLenum type,
                                GLuint id,
                                GLenum severity,
                                GLsizei length,
                                const GLchar* message,
                                const void* userParam) 
{
    // Ignore non-significant error/warning codes
    if (type == GL_DEBUG_TYPE_ERROR) {
        std::cerr << "GL CALLBACK: " << (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "") 
                  << " type = " << type 
                  << ", severity = " << severity 
                  << ", message = " << message << std::endl;
    }
}

int main(int argc, char** argv) 
{
     // Initialize random seed once. This will be removed by my own rng.
    static bool initialized = false;
    if (!initialized) {
        srand((time(nullptr))); // Seed the random number generator
        initialized = true;
    }

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);

    // Use OpenGL 3.3 core profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    Window window = {0};
    window.width = 1280;
    window.height = 720;

    window.window = SDL_CreateWindow("New evolution simulation", 
                                     SDL_WINDOWPOS_CENTERED, 
                                     SDL_WINDOWPOS_CENTERED, 
                                     window.width, window.height, 
                                     SDL_WINDOW_OPENGL);
    window.context = SDL_GL_CreateContext(window.window);

    SDL_GL_MakeCurrent(window.window, window.context);

    if (glewInit() != GLEW_OK) 
    {
        SDL_DestroyWindow(window.window);
        SDL_Quit();
        return -1;
    }

    // Enable debug output
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);
    // glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);

    // Setup ImGui
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
    io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport;

    ImGui_ImplSDL2_InitForOpenGL(window.window, window.context);
    ImGui_ImplOpenGL3_Init("#version 130");

#if 0
    // Vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Fragment shader
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragment_shader);

    // Shader program
    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertexShader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    // Clean up shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragment_shader);
#endif

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

    InputHandler* input = new InputHandler();
    while (running) 
    {

        input->mouseDelta = V2(0,0);
        while (SDL_PollEvent(&event)) 
        {
            ImGui_ImplSDL2_ProcessEvent(&event);

            switch(event.type)
            {

            case SDL_QUIT:
            {
                running = false;
            } break;

            case SDL_MOUSEMOTION:
            {
                // SDL reports mouse position and motion
                input->mousePos = V2(event.motion.x, event.motion.y);
                input->mouseDelta = V2(event.motion.xrel, event.motion.yrel);
            } break;

            }

            // We cheat and read input info from imgui. Why not.
            input->mouseScroll = io.MouseWheel;

            input->prevMouseDown[0] = input->mouseDown[0];
            input->mouseDown[0] = io.MouseDown[0];
            std::cout << io.MouseDelta.x << " " << io.MouseDelta.y << std::endl;
        }

        // Clear the screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Start ImGui Frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window.window);
        ImGui::NewFrame();

        // Create central dockspace for imgui.
        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::SetNextWindowBgAlpha(0.0f);

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        window_flags |= ImGuiWindowFlags_NoTitleBar |
                        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Dockspace main_", NULL, window_flags);
        ImGui::PopStyleVar(3);

        ImGuiID dockspace_id = ImGui::GetID("FullScreenDockSpace_");
        ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
        ImGui::DockSpace(dockspace_id, V2(0.0f, 0.0f), dockspace_flags);
        ImGui::End();

        ImGui::Begin("Hellow i set it  up agian");
        ImGui::Text("I just setup imgui");
        ImGui::End();

        // Draw the triangle
        glUseProgram(shader.program_handle);

        static float time = 0.0f;
        time += 0.016f;

        //cam.pos = V2(sinf(time), 0);
        UpdateCamera(&cam, window.width, window.height);

        glUniformMatrix3fv(transform_loc, 1, GL_FALSE, &cam.transform.m[0][0]);

        if(input->mouseDown[0])
        {
            cam.pos -= input->mouseDelta / (cam.scale);
        }

        Vec2 dims = V2(0.5, 0.5);
        Vec2 pos = V2(-0.5, -0.5);
        PushRect(&mesh, pos, pos+dims, V2(0,0), V2(0,0), 0xffaa77ff);
        BufferData(&mesh, GL_DYNAMIC_DRAW);

        Draw(&mesh);
        Clear(&mesh);

        // End ImGui frame
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap the screen buffers
        SDL_GL_SwapWindow(window.window);
    }

    // Cleanup
    SDL_GL_DeleteContext(window.context);
    SDL_DestroyWindow(window.window);
    SDL_Quit();

    return 0;
}
