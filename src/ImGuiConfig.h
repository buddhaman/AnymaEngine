#pragma once

#include "TimVectorMath.h"

#define IM_VEC2_CLASS_EXTRA                                                     \
        constexpr ImVec2(const Vec2& f) : x(f.x), y(f.y) {}                   \
        operator Vec2() const { return V2(x,y); }

#define IM_VEC4_CLASS_EXTRA                                                     \
        constexpr ImVec4(const Vec4& f) : x(f.x), y(f.y), z(f.z), w(f.w) {}   \
        operator Vec4() const { return V4(x,y,z,w); }