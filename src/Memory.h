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

// TODO: Track all allocations.
static inline void* 
HeapAllocSize(U64 size)
{
    return calloc(1, size);
}

#define HeapAlloc(type) (type*)HeapAllocSize(sizeof(type))

static inline void
HeapFree(void* ptr)
{
    free(ptr);
}

#define PushStruct(arena, type)               \
    (type *)PushMemory_(arena, sizeof(type)); 

// Placement new
#define PushNewStruct(arena, type)               \
    new (PushMemory_(arena, sizeof(type))) type;

#define PushArray(arena, type, nElements)                 \
    (type *)PushMemory_(arena, sizeof(type) * (nElements)); 

#define PushZeroArray(arena, type, nElements) \
    (type *)PushAndZeroMemory_(arena, sizeof(type) * (nElements));

MemoryArena *
CreateMemoryArena(size_t sizeInBytes);

MemoryArena *
CreateSubArena(MemoryArena *parent, size_t sizeInBytes);

void
DestroyArena(MemoryArena* arena);

static inline void 
ClearArena(MemoryArena *arena)
{
    arena->used = 0;
}

static inline void
ZeroArena(MemoryArena* arena)
{
    memset(arena->base, 0, arena->used);
}


template <typename T>
struct BittedMemoryPool
{
    T *base;
    I64 size;

    I64 maxBlocks;
    U32 *blocks; // Blocks of size 32.

    T *
    Alloc()
    {
        bool found = false;
        U32 blockIdx = 0;
        U32 bitIdx = 0;
        for(blockIdx = 0;
                blockIdx < maxBlocks;
                blockIdx++)
        {
            U32 block = blocks[blockIdx];
            if((~(block))!=0U)
            {
                bitIdx = 0;
                while((block)&1) 
                {
                    block>>=1;
                    bitIdx++;
                }
                blocks[blockIdx]|=(1<<bitIdx);
                found = true;
                break;
            }
        }

        if(!found) return nullptr;

        return &base[32*blockIdx + bitIdx];
    }

    void
    Free(T *element)
    {    
        I64 elementIdx = (((U8 *)element)-((U8 *)base))/sizeof(T);
        I64 blockIdx = elementIdx/32;
        I64 bitIdx = elementIdx-blockIdx*32;
        Assert(blockIdx < maxBlocks);
        Assert(bitIdx < 32);
        Assert(blocks[blockIdx]&(1<<bitIdx));
        blocks[blockIdx]&=~(1<<bitIdx);
    }
}; 

template <typename T>
BittedMemoryPool<T> *
CreateBittedMemoryPool(MemoryArena *arena, size_t maxBlocks)
{
    BittedMemoryPool<T> *pool = PushStruct(arena, BittedMemoryPool<T>);
    *pool = BittedMemoryPool<T>{};
    pool->maxBlocks = maxBlocks;
    pool->size = maxBlocks * 32;
    pool->base = PushArray(arena, T, maxBlocks*32);
    pool->blocks = PushArray(arena, U32, maxBlocks);
    memset(pool->base, 0, pool->size*sizeof(T));
    memset(pool->blocks, 0, maxBlocks * sizeof(U32));
    return pool;
}