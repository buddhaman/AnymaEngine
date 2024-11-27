#include <stdlib.h>

#include "Memory.h"

MemoryArena *
CreateMemoryArena(U64 size_in_bytes)
{
    U8 *total_memory = (U8 *)malloc(sizeof(MemoryArena) + size_in_bytes);
    MemoryArena *arena = (MemoryArena *)total_memory;
    Assert(total_memory);
    arena->base = total_memory + sizeof(MemoryArena);
    arena->used = 0;
    arena->size = size_in_bytes;
    return arena;
}

MemoryArena *
CreateSubArena(MemoryArena *parent, U64 size_in_bytes)
{
    MemoryArena *arena = PushStruct(parent, MemoryArena);
    arena->base = (U8 *)PushMemory_(parent, size_in_bytes);
    arena->used = 0;
    arena->size = size_in_bytes;
    return arena;
}

void
DestroyMemoryArena(MemoryArena* arena)
{
    free(arena);
}
