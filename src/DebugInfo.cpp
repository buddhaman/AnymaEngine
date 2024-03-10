#include <imgui.h>
#include <implot.h>

#include "DebugInfo.h"

void 
ImGuiChunkGrid(World* world)
{
    ImGui::SeparatorText("Chunks");
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