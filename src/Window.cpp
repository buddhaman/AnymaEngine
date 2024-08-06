#include <iostream>

#include <GL/glew.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>
#include <implot.h>

#include "Window.h"
#include "Lucide.h"

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

static inline void 
HandleInput(Window* window)
{
    InputHandler* input = &window->input;
    input->mouse_delta = V2(0,0);
    input->mouse_scroll = 0;
    memcpy(input->prev_keydown, input->keydown, InputAction_NumValues);
    SDL_Event event;
    while (SDL_PollEvent(&event)) 
    {
        ImGui_ImplSDL2_ProcessEvent(&event);

        switch(event.type)
        {

        case SDL_QUIT:
        {
            window->running = false;
        } break;

        case SDL_KEYDOWN:
        case SDL_KEYUP:
        {
            bool down = event.type==SDL_KEYDOWN;
            switch(event.key.keysym.scancode)
            {
                case SDL_SCANCODE_W: input->keydown[InputAction_W]=down; break;
            }
        } break;

        case SDL_MOUSEWHEEL:
        {
            input->mouse_scroll = 5*event.wheel.y; // Adjust zoom level based on scroll
        } break;

        case SDL_MOUSEMOTION:
        {
            // SDL reports mouse position and motion
            input->mouse_pos = V2(event.motion.x, event.motion.y);
            input->mouse_delta = V2(event.motion.xrel, event.motion.yrel);
        } break;

        }
    }

    // We cheat and read input info from imgui. Why not.
    ImGuiIO& io = ImGui::GetIO();
    input->prev_mousedown[0] = input->mousedown[0];
    input->mousedown[0] = io.MouseDown[0];
    input->mouse_clicked_at = io.MouseClickedPos[0];

    SDL_GetWindowSize(window->window, &window->width, &window->height);
}

void
DelayForFPS(Window* window)
{
    window->frame_end_time = SDL_GetPerformanceCounter();
    U64 freq = SDL_GetPerformanceFrequency();
    U64 elapsed = window->frame_end_time-window->frame_start_time;

    window->update_millis = 1000.0f*elapsed/(R32)freq;
    R32 millis_per_frame = 1000.0f/window->fps;
    R32 exact_delay = millis_per_frame - window->update_millis + window->carry_millis;
    I32 delay = floor(exact_delay);
    window->carry_millis = Max(0.0f, (R32)(exact_delay - delay));
    if(delay > 0)
    {
        SDL_Delay(delay);
    }
}

void 
WindowBegin(Window* window)
{
    window->frame_start_time = SDL_GetPerformanceCounter();

    HandleInput(window);
    // Clear the screen
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, window->width, window->height);

    // Start ImGui Frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window->window);
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
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, V2(0.0f, 0.0f));
    ImGui::Begin("Dockspace main_", NULL, window_flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("FullScreenDockSpace_");
    ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
    ImGui::DockSpace(dockspace_id, V2(0.0f, 0.0f), dockspace_flags);
    ImGui::End();
}

void 
WindowEnd(Window* window)
{
    // End ImGui frame
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Swap the screen buffers
    DelayForFPS(window);
    SDL_GL_SwapWindow(window->window);
}

Window* 
CreateWindow(int width, int height)
{
    Window* window = new Window();
    *window = {0};

    window->fps = 60;

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);

    // Use OpenGL 3.3 core profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4); // 4x MSAA

    window->width = width;
    window->height = height;

    window->window = SDL_CreateWindow("New evolution simulation", 
                                     SDL_WINDOWPOS_CENTERED, 
                                     SDL_WINDOWPOS_CENTERED, 
                                     window->width, window->height, 
                                     SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    window->context = SDL_GL_CreateContext(window->window);
    SDL_GL_MakeCurrent(window->window, window->context);

    // Disable VSync
    if (SDL_GL_SetSwapInterval(0) < 0) {
        std::cerr << "Warning: Unable to disable VSync! SDL Error: " << SDL_GetError() << std::endl;
    }

    if (glewInit() != GLEW_OK) 
    {
        SDL_DestroyWindow(window->window);
        SDL_Quit();
        return nullptr;
    }

    // Enable debug output
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);
    // glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);

    // Setup alpha blending globally
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Setup ImGui
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
    io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport;

    // Load imgui custom font.
    const char* default_font_path = "assets/Roboto-Regular.ttf";  
    float font_size = 18.0f;  
    ImFont* font = io.Fonts->AddFontFromFileTTF(default_font_path, font_size);
    io.Fonts->Build(); 
    io.FontDefault = font;

    const char* icon_font_path = "assets/lucide.ttf";  
    ImFontConfig font_config;
    font_config.MergeMode = true; 
    font_config.PixelSnapH = true; 

    // Merge icon characters into the default font
    static const ImWchar icon_ranges[] = { ICON_MIN_LC, ICON_MAX_LC, 0 }; 
    io.Fonts->AddFontFromFileTTF(icon_font_path, font_size*0.75f, &font_config, icon_ranges);
    io.Fonts->Build();

    ImGui_ImplSDL2_InitForOpenGL(window->window, window->context);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Setup ImPlot
    ImPlot::CreateContext();

    window->running = true;

    return window;
}

void DestroyWindow(Window* window)
{
    SDL_GL_DeleteContext(window->context);
    SDL_DestroyWindow(window->window);
    SDL_Quit();
}