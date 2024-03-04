#pragma once

#include <SDL.h>

struct Window
{
    int width;
    int height;
    SDL_Window* window;
    SDL_GLContext context;
};
