#pragma once

#include <SDL.h>

#include "InputHandler.h"

#undef CreateWindow

struct Window
{
    int width;
    int height;
    SDL_Window* window;
    SDL_GLContext context;
    InputHandler input;

    // This should not live in window.
    bool running;

    R32 carry_millis;
    R32 update_millis;

    U64 frame_end_time;
    U64 frame_start_time;

    R32 fps;
};

void
DelayForFPS(Window* window);

void 
WindowBegin(Window* window);

void 
WindowEnd(Window* window);

Window* 
CreateWindow(int width, int height);

void 
DestroyWindow(Window* window);