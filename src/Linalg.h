#pragma once

#include "AnymUtil.h"
#include "Memory.h"

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

struct VecR32
{
    I32 n;
    R32 *v;

    inline R32& operator[](int idx) { Assert(0 <= idx && idx < n); return v[idx]; }
    inline void Set(R32 value) { for(int i = 0; i < n; i++) { v[i] = value; } } 
    inline void Apply(R32 (*f)(R32 x)) { for(int i = 0; i < n; i++) { v[i] = f(v[i]); } } 
    inline void Apply(R32 (*f)(int idx, R32 x)) { for(int i = 0; i < n; i++) { v[i] = f(i, v[i]); } } 
    inline R32 Sum() { R32 sum = 0.0f; for(int i = 0; i < n; i++) { sum += v[i]; } return sum; } 
    inline R32 Len2() { R32 sum = 0.0f; for(int i = 0; i < n; i++) { sum += v[i]*v[i]; } return sum;  } 
    inline R32 Avg() { return Sum() / n; }
    inline void CopyFrom(VecR32 from) { Assert(n == from.n); memcpy(v, from.v, from.n*sizeof(R32)); }

    inline MatR32 ShapeAs(int w, int h, I64& offset)
    {
        Assert(offset+w*h <= n);
        MatR32 result;
        result.w = w;
        result.h = h;
        result.m = v+offset;
        offset += w*h;
        return result;
    }

    inline R32 StdDev() 
    {
        R32 mean = Avg();
        R32 sum = 0.0f;
        for(int i = 0; i < n; i++) {
            R32 val = v[i];
            sum += (val - mean) * (val - mean);
        }
        return sqrt(sum / n);
    }

    void
    PrintP(FILE *stream, int w=3, int p=2)
    {
        fprintf(stream, "| ");
        for(int r = 0; r < n; r++)
        {
            fprintf(stream, "%*.*f ", w, p, v[r]);
        }
        fprintf(stream, "|\n");
    }

    void AddNormal(R32 avg, R32 dev)
    {
        U32 pairs = n/2;

        for(U32 atPair = 0;
                atPair < pairs;
                atPair++)
        {
            Vec2 v_norm = RandomNormalPairDebug();
            v[atPair*2] += (avg + v_norm.x*dev);
            v[atPair*2+1] += (avg + v_norm.y*dev);
        }
        if(n%2==1)
        {
            v[n-1] += (RandomNormalPairDebug().x*dev);
        }
    }

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

static inline VecR32 
VecR32Create(int n)
{
    return {n, (R32*)malloc(n*sizeof(R32))};
}

static inline void
VecR32Destroy(VecR32 vec)
{
    free(vec.v);
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
    Assert(a.w == v.n); 
    Assert(result.n == a.h); 
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
