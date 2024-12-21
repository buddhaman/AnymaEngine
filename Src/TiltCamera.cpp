#include "TiltCamera.h"

void
UpdateTiltedCamera(TiltedCamera* camera, int screen_width, int screen_height)
{
    camera->size = V2((R32)screen_width, (R32)screen_height);
    camera->c = cosf(camera->angle);
    camera->s = sinf(camera->angle);
}

void
UpdateTiltedCameraScrollInput(TiltedCamera* camera, InputHandler* input)
{
    camera->scale *= powf(1.05f, input->mouse_scroll);
}

void
UpdateTiltedCameraDragInput(TiltedCamera* camera, InputHandler* input)
{
    if (input->mousedown[0]) // If left mouse button is pressed
    {
        // Mouse delta scaled to world space
        Vec2 diff = input->mouse_delta / camera->scale;

        R32 c = camera->c; // cos(theta)
        R32 s = camera->s; // sin(theta)

        // Adjust camera position for drag
        camera->pos.x -= diff.x;                 
        camera->pos.y += (diff.y / c*c);          
    }
}

void
TiltedCameraStopDragging(TiltedCamera* camera)
{
    camera->is_dragging = 0;
}

Vec3
TiltedMouseToWorld(TiltedCamera* camera, InputHandler* input, int screen_width, int screen_height)
{
    Vec2 normalized_mouse = V2(
        (2.0f * input->mouse_pos.x / screen_width) - 1.0f,
        1.0f - (2.0f * input->mouse_pos.y / screen_height)
    );

    // Scale by the camera's size and scale
    Vec2 screen_offset = normalized_mouse * (camera->size / camera->scale * 0.5f);

    R32 c = camera->c; // cos(angle)
    R32 s = camera->s; // sin(angle)

    // Map the screen-space coordinates to world space
    Vec3 world_pos;
    world_pos.x = camera->pos.x + screen_offset.x;          
    world_pos.y = camera->pos.y/c + screen_offset.y / c;   
    world_pos.z = 0;
    return world_pos;
}
