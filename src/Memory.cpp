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

MemoryArena *
CreateSubArena(MemoryArena *parent, U64 sizeInBytes)
{
    MemoryArena *arena = PushStruct(parent, MemoryArena);
    arena->base = (U8 *)PushMemory_(parent, sizeInBytes);
    arena->used = 0;
    arena->size = sizeInBytes;
    return arena;
}

void
DestroyMemoryArena(MemoryArena* arena)
{
    free(arena);
}
