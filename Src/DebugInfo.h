#pragma once

#include "World.h"

void 
ImGuiCellGrid(World* world);

void 
ImGuiCellBarGraph(World* world);

void 
ImGuiCellDistribution(World* world);

void
ImGuiMemoryArena(MemoryArena* arena, const char* name);

void 
ImGuiBrainVisualizer(Brain* brain);
