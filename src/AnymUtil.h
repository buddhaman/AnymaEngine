#pragma once

// TODO: Remove all dependencies on stl.
#include <stdint.h>
#include <iostream>
#include <cassert>

#include <math.h>

#include "TimVectorMath.h"

// Macros

#ifdef _NDEBUG
#define AssertMsg(...) do {} while(0)
#define Assert(...) do {} while(0)
#define InvalidCodePath(...) do {} while(0)
#else
#define AssertMsg(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "Assertion failed: " << (msg) << \
                         "\nFile: " << __FILE__ << \
                         "\nLine: " << __LINE__ << std::endl; \
            assert(cond); \
        } \
    } while (0)

#define Assert(cond) \
    do { \
        if (!(cond)) { \
            std::cerr << "Assertion failed: " << \
                         "\nFile: " << __FILE__ << \
                         "\nLine: " << __LINE__ << std::endl; \
            assert(cond); \
        } \
    } while (0)

#define InvalidCodePath() \
    do { \
        std::cerr << "Invalid code path reached: " << \
                        "\nFile: " << __FILE__ << \
                        "\nLine: " << __LINE__ << std::endl; \
        assert(false); \
         \
    } while (0)
#endif

#define ArraySize(array) (sizeof(array)/sizeof((array)[0]))

// Typedefs

typedef int8_t I8;
typedef uint8_t U8;
typedef int16_t I16;
typedef uint16_t U16;
typedef int32_t I32;
typedef uint32_t U32;
typedef int64_t I64;
typedef uint64_t U64;

typedef float R32;
typedef double R64;

static constexpr R32 R32_MAX = FLT_MAX;
static constexpr R64 R64_MAX = DBL_MAX;

// Functions
template <typename T>
T Max(T a, T b) { return a < b ? b : a; }

template <typename T>
T Min(T a, T b) { return a < b ? a : b; }

template <typename T>
T Clamp(T min, T val, T max)
{
    if(val < min) return min;
    if(val > max) return max;
    return val;
}

static inline float 
RandomR32Debug(float min, float max)
{
    float randFloat = (float)(rand()) / (float)(RAND_MAX);
    return randFloat * (max - min) + min;
}

static inline Vec2 
RandomVec2Debug(Vec2 min, Vec2 max)
{
    return V2(RandomR32Debug(min.x, max.x), RandomR32Debug(min.y, max.y));
}

static inline Vec2
RandomNormalPairDebug()
{
    R32 x, y, s;
    do
    {
        x = RandomR32Debug(-1.0f, 1.0f);
        y = RandomR32Debug(-1.0f, 1.0f);
        s = x*x + y*y;
    } while(s > 1.0);
    R32 factor = sqrtf(-2*logf(s)/s);
    return V2(factor*x, factor*y);
}

static inline Vec4
HexToVec4(U32 hex)
{
    Vec4 color;
    color.r = ((hex >> 24) & 0xFF) / 255.0f;
    color.g = ((hex >> 16) & 0xFF) / 255.0f;
    color.b = ((hex >> 8) & 0xFF) / 255.0f;
    color.a = (hex & 0xFF) / 255.0f;
    return color;
}

static inline uint32_t
Vec4ToHex(R32 r, R32 g, R32 b, R32 a)
{
    uint32_t hex = 0;

    uint8_t rb = (uint8_t)(roundf(r * 255.0f));
    uint8_t gb = (uint8_t)(roundf(g * 255.0f));
    uint8_t bb = (uint8_t)(roundf(b * 255.0f));
    uint8_t ab = (uint8_t)(roundf(a * 255.0f));

    hex |= (rb << 24);
    hex |= (gb << 16);
    hex |= (bb << 8);
    hex |= ab;

    return hex;
}
