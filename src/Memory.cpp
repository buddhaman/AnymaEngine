#include <stdlib.h>

#include "Memory.h"

MemoryArena *
CreateMemoryArena(U64 sizeInBytes)
{
    U8 *totalMemory = (U8 *)malloc(sizeof(MemoryArena) + sizeInBytes);
    MemoryArena *arena = (MemoryArena *)totalMemory;
    Assert(totalMemory);
    arena->base = totalMemory + sizeof(MemoryArena);
    arena->used = 0;
    arena->size = sizeInBytes;
    return arena;
}

void 
ClearArena(MemoryArena *arena)
{
    memset(arena->base, 0, arena->used);
    arena->used = 0;
}

#define PushStruct(arena, type)               \
    (type *)PushMemory_(arena, sizeof(type)); \
    // DebugOut(""#type" : %p", (void*)(arena->base+arena->used))
#define PushArray(arena, type, nElements)                 \
    (type *)PushMemory_(arena, sizeof(type) * nElements); \
    // DebugOut(""#type" : %p", (void*)(arena->base+arena->used))
#define PushAndZeroArray(arena, type, nElements)                 \
    (type *)PushAndZeroMemory_(arena, sizeof(type) * nElements); \
    // DebugOut(""#type" : %p", (void*)(arena->base+arena->used))
#define PushZeroArray(arena, type, nElements) \
    (type *)PushAndZeroMemory_(arena, sizeof(type) * nElements);

MemoryArena *
CreateSubArena(MemoryArena *parent, U64 sizeInBytes)
{
    MemoryArena *arena = PushStruct(parent, MemoryArena);
    arena->base = (U8 *)PushMemory_(parent, sizeInBytes);
    arena->used = 0;
    arena->size = sizeInBytes;
    return arena;
}
