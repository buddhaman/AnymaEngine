#pragma once

#include "AnymUtil.h"

#define String8Alloc(size) malloc(size)
#define String8Free(ptr) free(ptr)

struct String8
{
    U8* data;
    U32 length;
};

static inline String8 
Concat(String8 a, String8 b)
{
    String8 result;
    result.length = a.length + b.length;
    result.data = (U8*)String8Alloc(result.length);
    memcpy(result.data, a.data, a.length);
    memcpy(result.data+a.length, b.data, b.length);
    return result;
}

static inline U32
CStringLength(const char* c)
{
    U32 length = 0;
    while((c++)[0])
    {
        length++;
    }
    return length;
}

static inline const String8
CopyFromLiteral(const char* literal)
{
    String8 result;
    result.length = CStringLength(literal);
    result.data = (U8*)malloc(result.length+1);
    memcpy(result.data, literal, result.length);
    result.data[result.length] = 0;
    return result;
}

static inline void 
Destroy(String8 s)
{
    String8Free(s.data);
}