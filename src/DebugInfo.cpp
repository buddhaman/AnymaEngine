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
            ImGui::TableNextRow(); // Start a new row
            for(int x = 0; x < world->x_chunks; x++)
            {
                ImGui::TableNextColumn(); // Move to the next column
                Chunk* chunk = GetChunk(world, x, y); // You need to implement this function
                if (chunk != nullptr) 
                {
                    ImGui::Text("%zu", chunk->agent_indices.size); // Display the number in the cell
                } 
                else 
                {
                    ImGui::Text("Empty"); // Or handle empty chunks however you prefer
                }
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
        chunk_agents.PushBack(chunk->agent_indices.size);
    }
    ImPlot::SetNextAxesToFit();
    if(ImPlot::BeginPlot("Number of agents per chunk", V2(-1, 200), chunk_bar_plot_flags))
    { 
        ImPlot::PlotBars("Agents per chunk bar plot", chunk_agents.data, chunk_agents.size, 1);
        ImPlot::EndPlot();
    }
}

void 
ImGuiChunkDistribution(World* world, int n_bins)
{
    DynamicArray<U32> chunk_agents(world->chunks.size);
    DynamicArray<U32> bins(n_bins);
    bins.FillAndSetValue(0);

    ImPlotFlags chunk_bar_plot_flags = ImPlotFlags_NoBoxSelect | 
                        ImPlotFlags_NoInputs | 
                        ImPlotFlags_NoFrame | 
                        ImPlotFlags_NoLegend;

    for(int chunk_idx = 0; chunk_idx < world->chunks.size; chunk_idx++)
    {
        Chunk* chunk = &world->chunks[chunk_idx];
        chunk_agents.PushBack(chunk->agent_indices.size);
    }

    U32 min = chunk_agents.MinElement();
    U32 max = chunk_agents.MaxElement();

    // TODO: Is adding +0.1 the best way to solve rounding issues?
    float bin_size = (max-min+0.1)/((R32)n_bins);

    for(int val : chunk_agents)
    {
        int bin_idx = (int)floor((val-min)/bin_size);
        Assert(bin_idx >= 0);
        Assert(bin_idx < n_bins);
        bins[bin_idx]++;
    }

    ImPlot::SetNextAxesToFit();
    if(ImPlot::BeginPlot("Distribution of agents per chunk.", V2(-1, 200), chunk_bar_plot_flags))
    { 
        ImPlot::PlotBars("ChunkDistr", bins.data, bins.size, 1);
        ImPlot::EndPlot();
    }
    ImGui::Text("Minimum agents in a chunk: %u", min);
    ImGui::Text("Maximum agents in a chunk: %u", max);
}

