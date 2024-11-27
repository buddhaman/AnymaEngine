#pragma once

#include "World.h"

void 
ImGuiChunkGrid(World* world);

void 
ImGuiChunkBarGraph(World* world);

void 
ImGuiChunkDistribution(World* world);

void
ImGuiMemoryArena(MemoryArena* arena, const char* name);

void 
ImGuiBrainVisualizer(Brain* brain);
