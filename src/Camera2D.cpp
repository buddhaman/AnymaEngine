#include "Camera2D.h"

void
UpdateCamera(Camera2D *camera, int screen_width, int screen_height)
{
    R32 xScale = 2.0f*camera->scale/screen_width;
    R32 yScale = 2.0f*camera->scale/screen_height;
    camera->size = V2(screen_width / camera->scale, screen_height/camera->scale);
    camera->transform = M3TranslationAndScale(-camera->pos, xScale, yScale);
    camera->bounds = {camera->pos - camera->size/2.0f, camera->size};
}

void
UpdateCameraScrollInput(Camera2D *camera)
{

}

void
UpdateCameraDragInput(Camera2D *camera)
{
    
}

void
CameraStopDragging(Camera2D *camera)
{
    camera->isDragging = 0;
}

Vec2
MouseToWorld(Camera2D* camera, InputHandler* input, int screen_width, int screen_height)
{
    R32 xf = input->mouse_pos.x/((R32)screen_width);
    R32 yf = 1.0f - input->mouse_pos.y/((R32)screen_height);
    return camera->pos + camera->size*V2(xf, yf) - camera->size/2.0f;
}