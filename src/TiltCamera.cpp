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
    if(input->mousedown[0])
    {
        Vec2 diff = input->mouse_delta / (camera->scale);

        R32 c = camera->c; 
        R32 s = camera->s;

        camera->pos.x -= diff.x;

        camera->pos.y += c * diff.y + s * diff.y;
    }
}

void
TiltedCameraStopDragging(TiltedCamera* camera)
{
    camera->is_dragging = 0;
}

Vec2
TiltedMouseToWorld(TiltedCamera* camera, InputHandler* input, int screen_width, int screen_height)
{
    return V2(0,0);
}
