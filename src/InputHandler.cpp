#include <imgui.h>
#include "InputHandler.h"

bool IsKeyDown(InputHandler* input, InputAction action)
{
    return input->keydown[action];
}

bool IsMouseClicked(InputHandler* input, int mouse_idx)
{
    // TODO: Handle this ourselves instead of depending on imgui. Also this calculation is not perfect. 
    ImGuiIO& io = ImGui::GetIO();
    if(!io.MouseReleased[mouse_idx])
    {
        return false;
    }
    return (V2Len2(input->mouse_clicked_at - input->mouse_pos) < 8.0f);
}