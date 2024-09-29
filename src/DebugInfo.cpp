#include <imgui.h>
#include <implot.h>

#include "DebugInfo.h"

void 
ImGuiChunkGrid(World* world)
{
    if (ImGui::BeginTable("ChunksTable", world->x_chunks, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) 
    {
        for(int y = 0; y < world->y_chunks; y++)
        {
            ImGui::TableNextRow(); 
            for(int x = 0; x < world->x_chunks; x++)
            {
                ImGui::TableNextColumn(); 
                Chunk* chunk = GetChunk(world, x, y); 
                ImGui::Text("%zu", chunk->agent_indices.size); 
            }
        }
        ImGui::EndTable();
    }
}

void 
ImGuiChunkBarGraph(World* world)
{
    DynamicArray<U32> chunk_agents;
    ImPlotFlags chunk_bar_plot_flags = ImPlotFlags_NoBoxSelect | 
                        ImPlotFlags_NoInputs | 
                        ImPlotFlags_NoFrame | 
                        ImPlotFlags_NoLegend;

    for(int chunk_idx = 0; chunk_idx < world->chunks.size; chunk_idx++)
    {
        Chunk* chunk = &world->chunks[chunk_idx];
        chunk_agents.PushBack((U32)chunk->agent_indices.size);
    }
    ImPlot::SetNextAxesToFit();
    if(ImPlot::BeginPlot("Number of agents per chunk", V2(-1, 200), chunk_bar_plot_flags))
    { 
        ImPlot::PlotBars("Agents per chunk bar plot", chunk_agents.data, (int)chunk_agents.size, 1);
        ImPlot::EndPlot();
    }
}

void 
ImGuiChunkDistribution(World* world)
{
    DynamicArray<U32> chunk_agents(world->chunks.size);

    ImPlotFlags chunk_bar_plot_flags = ImPlotFlags_NoBoxSelect | 
                        ImPlotFlags_NoInputs | 
                        ImPlotFlags_NoFrame | 
                        ImPlotFlags_NoLegend;

    for(int chunk_idx = 0; chunk_idx < world->chunks.size; chunk_idx++)
    {
        Chunk* chunk = &world->chunks[chunk_idx];
        chunk_agents.PushBack((int)chunk->agent_indices.size);
    }

    U32 min = chunk_agents.MinElement();
    U32 max = chunk_agents.MaxElement();

    int n_bins = max-min+1;
    DynamicArray<U32> bins(n_bins);
    bins.FillAndSetValue(0);
    for(int val : chunk_agents)
    {
        int bin_idx = val - min;
        bins[bin_idx]++;
    }

    ImPlot::SetNextAxesToFit();
    if(ImPlot::BeginPlot("Distribution of agents per chunk.", V2(-1, 200), chunk_bar_plot_flags))
    { 
        ImPlot::SetupAxes("Number of Agents", "Number of Chunks");
        ImPlot::PlotBars("ChunkDistr", bins.data, (int)bins.size, 1);
        ImPlot::EndPlot();
    }
    ImGui::Text("Minimum agents in a chunk: %u", min);
    ImGui::Text("Maximum agents in a chunk: %u", max);
}

void 
ImGuiBrainVisualizer(Brain* brain)
{
    if(ImPlot::BeginPlot("Gene"))
    {
        ImPlot::PlotHeatmap("Gene Map", brain->weights.m, brain->weights.w, brain->weights.h, -2.0f, 2.0f);
        ImPlot::EndPlot();
    }
}

void
ImGuiMemoryArena(MemoryArena* arena, const char* name)
{
    R32 used = ((R32)arena->used / (1024.0f*1024.0f));
    R32 total = ((R32)arena->size / (1024.0f*1024.0f));
    ImGui::Text("%s : (%.2f / %.2f) mb used", name, used, total);
}
