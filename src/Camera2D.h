#pragma once

#include "AnymUtil.h"
#include "InputHandler.h"

struct Camera2D
{
    Vec2 pos = V2(0,0);
    Vec2 size = V2(0,0);
    R32 scale = 1.0f;
    Mat3 transform;
    bool isDragging;
};

void
UpdateCamera(Camera2D *camera, int screenWidth, int screenHeight);

void
UpdateCameraScrollInput(InputHandler input, Camera2D *camera);

void
UpdateCameraDragInput(InputHandler input, Camera2D *camera);

void
CameraStopDragging(Camera2D *camera);

Vec2
MouseToWorld(Camera2D* camera, InputHandler* input, int screen_width, int screen_height);
