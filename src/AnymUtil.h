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
T Abs(T v) { return v < 0 ? -v : v; }

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

constexpr Vec4 
ColorToVec4(U32 u32color)
{
    return {
        ((u32color >> 24) & 0xFF) / 255.0f,
        ((u32color >> 16) & 0xFF) / 255.0f,
        ((u32color >> 8) & 0xFF) / 255.0f,
        (u32color & 0xFF) / 255.0f
    };
}

constexpr U32
RGBAColor(U8 r, U8 g, U8 b, U8 a)
{
    return (r << 24) | (g << 16) | (b << 8) | a;
}

constexpr U32
HexToColor(U32 hex)
{
    U32 r = (hex >> 24) & 0xFF;  
    U32 g = (hex >> 16) & 0xFF; 
    U32 b = (hex >> 8) & 0xFF; 
    U32 a = hex & 0xFF;       
    
    return RGBAColor(r, g, b, a);
}

constexpr U32
Vec4ToColor(R32 r, R32 g, R32 b, R32 a)
{
    U32 hex = 0;

    U8 rb = (U8)((int)(r * 255.0f));
    U8 gb = (U8)((int)(g * 255.0f));
    U8 bb = (U8)((int)(b * 255.0f));
    U8 ab = (U8)((int)(a * 255.0f));

    return RGBAColor(rb, gb, bb, ab);
}

constexpr U32
Vec4ToColor(Vec4 color)
{
    return Vec4ToColor(color.r, color.g, color.b, color.a);
}

// h - hue is 0 - 360 (degrees)
inline static U32
HSVAToRGBA(R32 h, R32 s, R32 v, R32 a)
{
    R32 r = 0.0f, g = 0.0f, b = 0.0f;

    // Convert HSV to RGB
    R32 c = v * s; // Chroma
    R32 x = c * (1.0f - Abs(fmod(h / 60.0f, 2.0f) - 1.0f));
    R32 m = v - c;

    if (0.0f <= h && h < 60.0f) 
    {
        r = c;
        g = x;
        b = 0.0f;
    } 
    else if (60.0f <= h && h < 120.0f) 
    {
        r = x;
        g = c;
        b = 0.0f;
    } 
    else if (120.0f <= h && h < 180.0f) 
    {
        r = 0.0f;
        g = c;
        b = x;
    } 
    else if (180.0f <= h && h < 240.0f) 
    {
        r = 0.0f;
        g = x;
        b = c;
    } 
    else if (240.0f <= h && h < 300.0f) 
    {
        r = x;
        g = 0.0f;
        b = c;
    } else 
    {
        r = c;
        g = 0.0f;
        b = x;
    }

    r += m;
    g += m;
    b += m;

    return Vec4ToColor(r, g, b, a);
}

#ifndef COLORS_H
#define COLORS_H

// Common colors defined globally
constexpr U32 Color_Red        = RGBAColor(255, 0, 0, 255);
constexpr U32 Color_Green      = RGBAColor(0, 255, 0, 255);
constexpr U32 Color_Blue       = RGBAColor(0, 0, 255, 255);
constexpr U32 Color_Yellow     = RGBAColor(255, 255, 0, 255);
constexpr U32 Color_Cyan       = RGBAColor(0, 255, 255, 255);
constexpr U32 Color_Magenta    = RGBAColor(255, 0, 255, 255);
constexpr U32 Color_White      = RGBAColor(255, 255, 255, 255);
constexpr U32 Color_Black      = RGBAColor(0, 0, 0, 255);
constexpr U32 Color_Gray       = RGBAColor(128, 128, 128, 255);
constexpr U32 Color_Orange     = RGBAColor(255, 165, 0, 255);
constexpr U32 Color_Purple     = RGBAColor(128, 0, 128, 255);
constexpr U32 Color_Brown      = RGBAColor(165, 42, 42, 255);
constexpr U32 Color_Pink       = RGBAColor(255, 192, 203, 255);
constexpr U32 Color_Lime       = RGBAColor(0, 255, 0, 255);
constexpr U32 Color_Aqua       = RGBAColor(0, 255, 255, 255);
constexpr U32 Color_Silver     = RGBAColor(192, 192, 192, 255);
constexpr U32 Color_Gold       = RGBAColor(255, 215, 0, 255);
constexpr U32 Color_Indigo     = RGBAColor(75, 0, 130, 255);
constexpr U32 Color_Violet     = RGBAColor(238, 130, 238, 255);

#endif