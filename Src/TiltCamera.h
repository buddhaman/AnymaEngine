#pragma once

#include "AnymUtil.h"
#include "InputHandler.h"
#include "TMath.h"

// The tilted camera translates takes the world coordinates in 3D and translates them to a position (x, y) in screen coordinates.
// Scale is a simple multiplication factor from world scale to screen pixel scale. 
struct TiltedCamera
{
    Vec3 pos = V3(0,0,0);
    Vec2 size = V2(0,0);

    // This one is read-only. Useful to have. Bounds in the 2D world. So intersection of view beam and ground plane.
    Rect bounds;

    R32 scale = 1.0f;
    R32 angle = 0.0f; // Tilt angle in radians, where 0 is top down.

    R32 c;  // Cosine of angle
    R32 s;  // Sine of angle

    bool is_dragging;
};

void
UpdateTiltedCamera(TiltedCamera* camera, int screen_width, int screen_height);

void
UpdateTiltedCameraScrollInput(TiltedCamera* camera, InputHandler* input);

void
UpdateTiltedCameraDragInput(TiltedCamera* camera, InputHandler* input);

void
TiltedCameraStopDragging(TiltedCamera* camera);

Vec3
TiltedMouseToWorld(TiltedCamera* camera, InputHandler* input, int screen_width, int screen_height);

static inline R32
GetScaled(TiltedCamera* camera, R32 s)
{
    return s*camera->scale;
}

static inline R32
GetX(TiltedCamera* camera, Vec3 pos)
{
    return (pos.x - camera->pos.x) * camera->scale;
}

static inline R32
GetY(TiltedCamera* camera, Vec3 pos)
{
    R32 projected_y = pos.y * camera->c - pos.z * camera->s;
    R32 ty = (projected_y - camera->pos.y*camera->c) * camera->scale;
    return ty;
}

static inline Vec2
GetVec2(TiltedCamera* camera, Vec3 pos)
{
    return {GetX(camera, pos), GetY(camera, pos)};
}