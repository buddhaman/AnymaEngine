#pragma once

#include "AnymUtil.h"

static constexpr U64 KiloBytes(U64 n)
{
    return n * 1024ULL;
}

static constexpr U64 MegaBytes(U64 n)
{
    return 1024ULL * KiloBytes(n);
}

static constexpr U64 GigaBytes(U64 n)
{
    return 1024ULL * MegaBytes(n);
}

struct MemoryArena
{
    U8 *base;
    I64 used;
    I64 size;
};

static inline void*
PushMemory_(MemoryArena* arena, size_t size)
{
    arena->used += size;
    Assert(arena->used <= arena->size);
    return arena->base + arena->used - size;
}

static inline void *
PushAndZeroMemory_(MemoryArena *arena, size_t size)
{
    void *memory = PushMemory_(arena, size);
    memset(memory, 0, size);
    return memory;
}

#define PushStruct(arena, type)               \
    (type *)PushMemory_(arena, sizeof(type)); 

// Placement new
#define PushNewStruct(arena, type)               \
    new (PushMemory_(arena, sizeof(type))) type;

#define PushArray(arena, type, nElements)                 \
    (type *)PushMemory_(arena, sizeof(type) * nElements); 

#define PushAndZeroArray(arena, type, nElements)                 \
    (type *)PushAndZeroMemory_(arena, sizeof(type) * nElements); 

#define PushZeroArray(arena, type, nElements) \
    (type *)PushAndZeroMemory_(arena, sizeof(type) * nElements);

MemoryArena *
CreateMemoryArena(size_t sizeInBytes);

void 
ClearArena(MemoryArena *arena);

MemoryArena *
CreateSubArena(MemoryArena *parent, size_t sizeInBytes);