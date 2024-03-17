#pragma once

#include "AnymUtil.h"
#include "Memory.h"

struct VecR32
{
    I32 n;
    R32 *v;

    inline void Apply(R32 (*f)(R32 x)) { for(int i = 0; i < n; i++) { v[i] = f(v[i]); } } 
    inline void Apply(R32 (*f)(int idx, R32 x)) { for(int i = 0; i < n; i++) { v[i] = f(i, v[i]); } } 
    inline R32 Sum() { R32 sum = 0.0f; for(int i = 0; i < n; i++) { sum += v[i]; } return sum; } 
    inline R32 Len2() { R32 sum = 0.0f; for(int i = 0; i < n; i++) { sum += v[i]*v[i]; } return sum;  } 
    inline R32 Avg() { return Sum() / n; }
    inline R32& operator[](int idx) { Assert(0 <= idx && idx < n); return v[idx]; }
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

static inline void
VecR32Add(VecR32 result, VecR32 a, VecR32 b)
{
    Assert(result.n == a.n);
    Assert(result.n == b.n);
    for(int i = 0; i < a.n; i++)
    {
        result[i] = a[i] + b[i];
    }
}

static inline void
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

    R32& Elem(int x, int y)
    {
        assert(0 <= x && x < w && 0 <= y && y < h);
        return m[x+y*w];
    }
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

static inline void 
MatR32Mul(MatR32 result, MatR32 a, MatR32 b)
{
    Assert(a.w == b.h); 
    Assert(result.w == b.w);
    Assert(result.h == a.h); 

    for(int i = 0; i < a.h; i++)
    {
        for(int j = 0; j < b.w; j++)
        {
            R32 sum = 0.0f;
            for(int k = 0; k < a.w; k++)
            {
                sum += a.m[i * a.w + k] * b.m[k * b.w + j];
            }
            result.m[i * result.w + j] = sum;
        }
    }
}

// Matrix-vector multiplication
static inline void 
MatR32VecMul(VecR32 result, MatR32 a, VecR32 v)
{
    Assert(a.w == v.n); // Ensure the matrix and vector can be multiplied
    Assert(result.n == a.h); // Ensure result vector has correct dimension
    for(int i = 0; i < a.h; i++)
    {
        R32 sum = 0.0f;
        for(int j = 0; j < a.w; j++)
        {
            sum += a.m[i * a.w + j] * v.v[j];
        }
        result.v[i] = sum;
    }
}
