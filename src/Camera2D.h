#pragma once

#include "AnymUtil.h"
#include "InputHandler.h"
#include "TMath.h"

struct Camera2D
{
    Vec2 pos = V2(0,0);
    Vec2 size = V2(0,0);

    // This one is read-only. Useful to have.
    Rect bounds;

    R32 scale = 1.0f;
    Mat3 transform;
    bool isDragging;
};

void
UpdateCamera(Camera2D *camera, int screenWidth, int screenHeight);

void
UpdateCameraScrollInput(Camera2D *camera, InputHandler* input);

void
UpdateCameraDragInput(Camera2D *camera, InputHandler* input);

void
CameraStopDragging(Camera2D *camera);

Vec2
MouseToWorld(Camera2D* camera, InputHandler* input, int screen_width, int screen_height);
