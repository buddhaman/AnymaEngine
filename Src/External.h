#pragma once

#define SDL_MAIN_HANDLED
#define SDL_STATIC

#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#include "TimVectorMath.h"
#define IMGUI_USER_CONFIG "ImGuiConfig.h"

#include <SDL.h>
#include <gl3w.h>
#include <imgui.h>
#include <implot.h>

#include "gl3w.cpp"