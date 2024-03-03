#pragma once

#include "AnymUtil.h"
#include "Memory.h"

// Simple std::vector without slow debug build and slightly different implementation. No new/delete.
template <typename T>
struct Array
{
    U64 size = 0;
    U64 capacity = 0;
    T *data = nullptr;

    inline void PushBack(const T& t) { Assert(size < capacity); data[size++] = t; }
    inline T* PushBack() { Assert(size < capacity); return data+size++; }
    inline T& operator[](int idx) { Assert(idx >=0 && idx < size); return data[idx];}
    inline T& At(int idx) { Assert(idx >=0 && idx < size); return data[idx];}
    inline void Fill() {size = capacity;}
    inline void Clear() { size = 0;}

    inline void Swap(int idx0, int idx1) { T tmp = data[idx0]; data[idx0] = data[idx1]; data[idx1] = tmp; }

    inline void RemoveIndexUnordered(int idx) 
    {
        Assert(idx >= 0 && idx < size);
        if(idx!=size-1)
        {
            data[idx] = data[size-1];
        }
        this->size--;
    }

    inline void RemoveIndex(int idx)
    {
        Assert(idx >= 0 && idx < size);
        if(idx!=size-1)
        {
            memmove(data+idx, data+idx+1, sizeof(T)*(size-1-idx));
        }
        size--;
    }

    inline int IndexOf(T element)
    {
        for(int idx = 0; idx < size; idx++)
        {
            if(data[idx]==element) 
            {
                return idx;
            }
        }
        return -1;
    }

    inline int RemoveUnordered(T element)
    {
        int idx;
        for(idx = 0; idx < size; idx++)
        {
            if(data[idx]==element) 
            {
                RemoveIndexUnordered(idx);
                return idx;
            }
        }
        InvalidCodePath();
    }

};

template <typename T>
struct DynamicArray : public Array<T>
{
    using Array<T>::size;
    using Array<T>::capacity;
    using Array<T>::data;

    // I actually do not care if these are deleted or not.
    DynamicArray(const DynamicArray&) = delete;
    DynamicArray& operator=(const DynamicArray&) = delete;

    DynamicArray()
    {
        size = capacity = 0; data = nullptr;
    }

    ~DynamicArray()
    {
        free(data);
    }

    inline bool GrowCapacity(U64 toCapacity) 
    {
        capacity = toCapacity;
        data = (T*)realloc(data, capacity*sizeof(T));
        if(!data)
        {
            // When malloc fails the program is terminated if you ask me. So
            // this will result in a crash and thats fine.
            std::cerr << "No memory allocated in GrowCapacity" << std::endl;
            return false;
        }
        return true;
    }

    inline T* IncreaseSize(int toSize)
    {
        if(toSize > capacity)
        {
            U64 newSize = toSize > 1000 ? (toSize + toSize/2) : (toSize * 2);
            GrowCapacity(newSize);
        }
        size = toSize;
        return data+size-1;
    }

    inline void PushBack(const T& t) { T* newT = IncreaseSize(size+1) ; *newT = t; }
    inline T* PushBack() { T* newT = IncreaseSize(size+1); return newT; }
};

template <typename T> 
Array<T>
CreateArray(MemoryArena* arena, int capacity)
{
    Array<T> array = {0};
    array.capacity = capacity;
    array.data = (T *)PushAndZeroMemory_(arena, sizeof(T)*capacity);
    return array;
}

template <typename T> Array<T>
CreateArray_(void *dataMemory, int capacity)
{
    Array<T> array = {0};
    array.capacity = capacity;
    memset(dataMemory, 0, sizeof(T)*capacity);
    array.data = (T *)dataMemory;
    return array;
}

