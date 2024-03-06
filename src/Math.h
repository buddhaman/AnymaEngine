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
    Vec2 direction;
};

// Assume ray is normalized
static bool
RayCircleIntersect(Ray ray, Circle circle, Vec2* intersection)
{
    //Vec2 intersection = V2(0,0);
    Vec2 diff = circle.pos-ray.pos;
    R32 l2 = V2Len2(diff);

    if(l2 < circle.radius*circle.radius)
    {
        // Ray is inside the circle. 
        *intersection = ray.pos;
        return true;
    }

    R32 dp = V2Dot(ray.direction, diff);
    // Circle is behind the ray.
    if(dp < 0) return false;

    Vec2 nearest_to_center = ray.pos + ray.direction*dp;

    R32 nearest_d2 = V2Len2(nearest_to_center - circle.pos);
    if(nearest_d2 < circle.radius*circle.radius)
    {
        // Intersection
        R32 entry = sqrtf(circle.radius*circle.radius - nearest_d2);
        *intersection = nearest_to_center - entry*ray.direction;
        return true;
    }

    return false;
}
