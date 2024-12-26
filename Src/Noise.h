#pragma once

#include "AnymUtil.h"

struct PerlinNoise 
{
    int seed;
};

int 
Noise2(PerlinNoise* noise, int x, int y)
{
    static int perlin_hash[] = {208,34,231,213,32,248,233,56,161,78,24,140,71,48,140,254,245,255,247,247,40,
                        185,248,251,245,28,124,204,204,76,36,1,107,28,234,163,202,224,245,128,167,204,
                        9,92,217,54,239,174,173,102,193,189,190,121,100,108,167,44,43,77,180,204,8,81,
                        70,223,11,38,24,254,210,210,177,32,81,195,243,125,8,169,112,32,97,53,195,13,
                        203,9,47,104,125,117,114,124,165,203,181,235,193,206,70,180,174,0,167,181,41,
                        164,30,116,127,198,245,146,87,224,149,206,57,4,192,210,65,210,129,240,178,105,
                        228,108,245,148,140,40,35,195,38,58,65,207,215,253,65,85,208,76,62,3,237,55,89,
                        232,50,217,64,244,157,199,121,252,90,17,212,203,149,152,140,187,234,177,73,174,
                        193,100,192,143,97,53,145,135,19,103,13,90,135,151,199,91,239,247,33,39,145,
                        101,120,99,3,186,86,99,41,237,203,111,79,220,135,158,42,30,154,120,67,87,167,
                        135,176,183,191,253,115,184,21,233,58,129,233,142,39,128,211,118,137,139,255,
                        114,20,218,113,154,27,127,246,250,1,8,198,250,209,92,222,173,21,88,102,219};
    int hashInput = x + noise->seed * 31 + y * 57;
    hashInput ^= (hashInput >> 13); 
    return perlin_hash[(hashInput & 255)]; 
}

R32 
LinearInter(R32 x, R32 y, R32 s)
{
    return x + s * (y - x);
}

R32 
SmoothInter(R32 x, R32 y, R32 s)
{
    return LinearInter(x, y, s * s * (3 - 2 * s));
}

R32 
Noise2D(PerlinNoise* noise, R32 x, R32 y)
{
    int xInt = static_cast<int>(x);
    int yInt = static_cast<int>(y);
    R32 xFrac = x - static_cast<R32>(xInt);
    R32 yFrac = y - static_cast<R32>(yInt);
    int s = Noise2(noise, xInt, yInt);
    int t = Noise2(noise, xInt + 1, yInt);
    int u = Noise2(noise, xInt, yInt + 1);
    int v = Noise2(noise, xInt + 1, yInt + 1);
    R32 low = SmoothInter(static_cast<R32>(s), static_cast<R32>(t), xFrac);
    R32 high = SmoothInter(static_cast<R32>(u), static_cast<R32>(v), xFrac);
    return SmoothInter(low, high, yFrac);
}

R32 
Perlin2D(PerlinNoise* noise, R32 x, R32 y, R32 freq, int depth)
{
    R32 xa = x * freq;
    R32 ya = y * freq;
    R32 amp = 1.0f;
    R32 fin = 0.0f;
    R32 div = 0.0f;

    for (int i = 0; i < depth; i++)
    {
        div += 256.0f * amp;
        fin += Noise2D(noise, xa, ya) * amp;
        amp *= 0.5f;
        xa *= 2.0f;
        ya *= 2.0f;
    }

    return fin / div;
}

#if 0
int main(int argc, char *argv[])
{
    PerlinNoise noise = {42}; // Example seed

    int x, y;
    for (y = 0; y < 4000; y++)
        for (x = 0; x < 4000; x++)
            Perlin2D(&noise, x, y, 0.1f, 4);

    return 0;
}
#endif
