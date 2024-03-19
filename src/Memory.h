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
    (type *)PushMemory_(arena, sizeof(type) * (nElements)); 

#define PushZeroArray(arena, type, nElements) \
    (type *)PushAndZeroMemory_(arena, sizeof(type) * (nElements));

MemoryArena *
CreateMemoryArena(size_t sizeInBytes);

void 
ClearArena(MemoryArena *arena);

MemoryArena *
CreateSubArena(MemoryArena *parent, size_t sizeInBytes);

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
        U32 blockIdx = 0;
        U32 bitIdx = 0;
        for(blockIdx = 0;
                blockIdx < this->maxBlocks;
                blockIdx++)
        {
            U32 block = this->blocks[blockIdx];
            if((~(block))!=0U)
            {
                bitIdx = 0;
                while((block)&1) 
                {
                    block>>=1;
                    bitIdx++;
                }
                this->blocks[blockIdx]|=(1<<bitIdx);
                break;
            }
        }
        return &this->base[32*blockIdx + bitIdx];
    }

    void
    Free(T *element)
    {    
        I64 elementIdx = (((U8 *)element)-((U8 *)base))/sizeof(T);
        I64 blockIdx = elementIdx/32;
        I64 bitIdx = elementIdx-blockIdx*32;
        Assert(blockIdx < this->maxBlocks);
        Assert(bitIdx < 32);
        Assert(this->blocks[blockIdx]&(1<<bitIdx));
        this->blocks[blockIdx]&=~(1<<bitIdx);
    }
}; 

template <typename T>
BittedMemoryPool<T> *
CreateMemoryPool(MemoryArena *arena, size_t maxBlocks)
{
    BittedMemoryPool<T> *pool = PushStruct(arena, BittedMemoryPool<T>);
    *pool = BittedMemoryPool<T>{};
    pool->maxBlocks = maxBlocks;
    pool->sizeInBytes = sizeof(T) * maxBlocks * 32;
    pool->base = PushArray(arena, U8, pool->sizeInBytes);
    pool->blocks = PushArray(arena, U32, maxBlocks);
    memset(pool->base, 0, pool->sizeInBytes);
    memset(pool->blocks, 0, maxBlocks * sizeof(U32));
    return pool;
}