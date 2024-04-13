#pragma once

#include "AnymUtil.h"

#define String8Alloc(size) malloc(size)
#define String8Free(ptr) free(ptr)

struct String8
{
    U8* data;
    U32 length;
};

String8 
Concat(String8 a, String8 b)
{
    String8 result;
    result.length = a.length + b.length;
    result.data = (U8*)String8Alloc(result.length);
    memcpy(result.data, a.data, a.length);
    memcpy(result.data+a.length, b.data, b.length);
    return result;
}

void 
Destroy(String8 s)
{
    String8Free(s.data);
}