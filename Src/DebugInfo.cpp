#include <imgui.h>
#include <implot.h>

#include "DebugInfo.h"

void 
ImGuiCellGrid(World* world)
{
    if (ImGui::BeginTable("CellsTable", world->x_cells, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) 
    {
        for(int y = 0; y < world->y_cells; y++)
        {
            ImGui::TableNextRow(); 
            for(int x = 0; x < world->x_cells; x++)
            {
                ImGui::TableNextColumn(); 
                Cell* cell = GetCell(world, x, y); 
                ImGui::Text("%zu", cell->agent_indices.size); 
            }
        }
        ImGui::EndTable();
    }
}

void 
ImGuiCellBarGraph(World* world)
{
    DynamicArray<U32> cell_agents;
    ImPlotFlags cell_bar_plot_flags = ImPlotFlags_NoBoxSelect | 
                        ImPlotFlags_NoInputs | 
                        ImPlotFlags_NoFrame | 
                        ImPlotFlags_NoLegend;

    for(int cell_idx = 0; cell_idx < world->cells.size; cell_idx++)
    {
        Cell* cell = &world->cells[cell_idx];
        cell_agents.PushBack((U32)cell->agent_indices.size);
    }
    ImPlot::SetNextAxesToFit();
    if(ImPlot::BeginPlot("Number of agents per cell", V2(-1, 200), cell_bar_plot_flags))
    { 
        ImPlot::PlotBars("Agents per cell bar plot", cell_agents.data, (int)cell_agents.size, 1);
        ImPlot::EndPlot();
    }
}

void 
ImGuiCellDistribution(World* world)
{
    DynamicArray<U32> cell_agents(world->cells.size);

    ImPlotFlags cell_bar_plot_flags = ImPlotFlags_NoBoxSelect | 
                        ImPlotFlags_NoInputs | 
                        ImPlotFlags_NoFrame | 
                        ImPlotFlags_NoLegend;

    for(int cell_idx = 0; cell_idx < world->cells.size; cell_idx++)
    {
        Cell* cell = &world->cells[cell_idx];
        cell_agents.PushBack((int)cell->agent_indices.size);
    }

    U32 min = cell_agents.MinElement();
    U32 max = cell_agents.MaxElement();

    int n_bins = max-min+1;
    DynamicArray<U32> bins(n_bins);
    bins.FillAndSetValue(0);
    for(int val : cell_agents)
    {
        int bin_idx = val - min;
        bins[bin_idx]++;
    }

    ImPlot::SetNextAxesToFit();
    if(ImPlot::BeginPlot("Distribution of agents per cell.", V2(-1, 200), cell_bar_plot_flags))
    { 
        ImPlot::SetupAxes("Number of Agents", "Number of Cells");
        ImPlot::PlotBars("CellDistr", bins.data, (int)bins.size, 1);
        ImPlot::EndPlot();
    }
    ImGui::Text("Minimum agents in a cell: %u", min);
    ImGui::Text("Maximum agents in a cell: %u", max);
}

void 
ImGuiBrainVisualizer(Brain* brain)
{
    // if(ImPlot::BeginPlot("Gene"))
    // {
    //     ImPlot::PlotHeatmap("Gene Map", brain->weights.m, brain->weights.w, brain->weights.h, -2.0f, 2.0f);
    //     ImPlot::EndPlot();
    // }
}

void
ImGuiMemoryArena(MemoryArena* arena, const char* name)
{
    R32 used = ((R32)arena->used / (1024.0f*1024.0f));
    R32 total = ((R32)arena->size / (1024.0f*1024.0f));
    ImGui::Text("%s : (%.2f / %.2f) mb used", name, used, total);
}
