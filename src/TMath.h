#pragma once

#include "TimVectorMath.h"
#include "AnymUtil.h"

struct Rect
{
    Vec2 pos;
    Vec2 dims;
};

struct Circle
{
    Vec2 pos;
    R32 radius;
};

struct Ray
{
    Vec2 pos;
    Vec2 dir;
};

struct RayCollision
{
    Vec2 intersection;
    R32 distance;
};

// Assume ray is normalized
static inline bool
RayCircleIntersect(Ray ray, Circle circle, RayCollision* col_info)
{
    //Vec2 intersection = V2(0,0);
    Vec2 diff = circle.pos-ray.pos;
    R32 l2 = V2Len2(diff);

    if(l2 < circle.radius*circle.radius)
    {
        // Ray is inside the circle. 
        col_info->intersection = ray.pos;
        col_info->distance = 0.0f;
        return true;
    }

    R32 dp = V2Dot(ray.dir, diff);
    // Circle is behind the ray.
    if(dp < 0) return false;

    Vec2 nearest_to_center = ray.pos + ray.dir*dp;

    R32 nearest_d2 = V2Len2(nearest_to_center - circle.pos);
    if(nearest_d2 < circle.radius*circle.radius)
    {
        // Intersection
        R32 entry = sqrtf(circle.radius*circle.radius - nearest_d2);
        col_info->intersection = nearest_to_center - entry*ray.dir;
        col_info->distance = dp - entry;
        return true;
    }

    return false;
}

static inline bool
InBounds(Rect bounds, Vec2 point)
{
    if(point.x < bounds.pos.x || point.x > bounds.pos.x+bounds.dims.x || 
        point.y < bounds.pos.y || point.y > bounds.pos.y+bounds.dims.y)
    {
        return false;
    }
    return true;
}

static inline R32 
NormalizeAngle(R32 angle) 
{
    R32 twoPi = 2.0f * (R32)M_PI;
    angle = fmod(angle + (R32)M_PI, twoPi);  // Shift the range to [0, 2π) by adding π
    if (angle < 0) 
    {
        angle += twoPi;  // Ensure positive result in [0, 2π)
    }
    return angle - (R32)M_PI;  // Shift back to [-π, π]
}


