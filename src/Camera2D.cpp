#include "Camera2D.h"

void
UpdateCamera(Camera2D *camera, int screenWidth, int screenHeight)
{
    R32 xScale = 2.0f*camera->scale/screenWidth;
    R32 yScale = 2.0f*camera->scale/screenHeight;
    camera->size = V2(camera->scale*screenWidth, camera->scale*screenHeight);
    camera->transform = M3TranslationAndScale(-camera->pos, xScale, -yScale);
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