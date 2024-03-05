#pragma once

// TODO: Remove all dependencies on stl.
#include <stdint.h>
#include <iostream>
#include <cassert>

// Includes

#include "TimVectorMath.h"

// Macros

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

// Functions

static float GetRandomR32Debug(float min, float max)
{
    float randFloat = (float)(rand()) / (float)(RAND_MAX);
    return randFloat * (max - min) + min;
}

template <typename T>
T Max(T a, T b) { return a < b ? b : a; }

template <typename T>
T Min(T a, T b) { return a < b ? a : b; }
