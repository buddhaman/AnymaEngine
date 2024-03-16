#pragma once

#include "AnymUtil.h"

struct MatR32
{
    I32 w;
    I32 h;
    R32 *m;
};

typedef struct
{
    I32 n;
    R32 *v;
} VecR32;