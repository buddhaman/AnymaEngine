#pragma once

#include "AnymUtil.h"
#include "Memory.h"

struct VecR32
{
    I32 n;
    R32 *v;

    inline void Apply(R32 (*f)(R32 x)) { for(int i = 0; i < n; i++) { v[i] = f(v[i]); } } 
    inline R32 Sum() { R32 sum = 0.0f; for(int i = 0; i < n; i++) { sum += v[i]; } return sum; } 
    inline R32 Len2() { R32 sum = 0.0f; for(int i = 0; i < n; i++) { sum += v[i]*v[i]; } return sum;  } 
    inline R32 Avg() { return Sum() / n; }
    inline R32& operator[](int idx) { return v[idx]; }
};

static inline VecR32 
VecR32Create(int n, R32* data)
{
    return {n, data};
}

static inline VecR32 
VecR32Create(MemoryArena* arena, int n)
{
    R32* data = PushZeroArray(arena, R32, n);
    return VecR32Create(n, data);
}

void
VecR32Add(VecR32 result, VecR32 a, VecR32 b)
{
    Assert(result.n == a.n);
    Assert(result.n == b.n);
    for(int i = 0; i < a.n; i++)
    {
        result[i] = a[i] + b[i];
    }
}

void
VecR32Mul(VecR32 result, VecR32 a, VecR32 b)
{
    Assert(result.n == a.n);
    Assert(result.n == b.n);
    for(int i = 0; i < a.n; i++)
    {
        result[i] = a[i] * b[i];
    }
}

struct MatR32
{
    I32 w;
    I32 h;
    R32 *m;
};

static inline MatR32 
MatR32Create(int w, int h, R32* data) 
{
    return {w, h, data};
}

static inline MatR32 
MatR32Create(MemoryArena* arena, int w, int h)
{
    R32* data = PushZeroArray(arena, R32, w*h);
    return MatR32Create(w, h, data);
}