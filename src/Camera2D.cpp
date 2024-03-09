#include "Camera2D.h"

void
UpdateCamera(Camera2D *camera, int screen_width, int screen_height)
{
    R32 xScale = 2.0f*camera->scale/screen_width;
    R32 yScale = 2.0f*camera->scale/screen_height;
    camera->size = V2(screen_width / camera->scale, screen_height/camera->scale);
    camera->transform = M3TranslationAndScale(-camera->pos, xScale, yScale);
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