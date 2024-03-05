#pragma once

#include <SDL.h>

#include "InputHandler.h"

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
};

Window* 
CreateWindow(int width, int height);

void 
DestroyWindow(Window* window);

void 
WindowBegin(Window* window);

void 
WindowEnd(Window* window);