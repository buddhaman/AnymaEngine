#pragma once

// TODO: REMOVE
#include <functional>

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

    // TODO: Dont be dependent on standard library. Get rid of STDs.
    inline void Sort(const std::function<int(T, T)>& compare)
    {
        if(size == 0)
        {
            return;
        }
        Quicksort(0, size-1, compare);
    }

    // Iterator support
    struct Iterator {
        T* ptr;
        Iterator(T* ptr) : ptr(ptr) {}
        bool operator!=(const Iterator& other) const { return ptr != other.ptr; }
        const T& operator*() const { return *ptr; }
        T& operator*() { return *ptr; }
        Iterator& operator++() { ++ptr; return *this; }  // Prefix increment
    };
    Iterator begin() { return Iterator(data); }
    Iterator end() { return Iterator(data + size); }

    private:
    void Quicksort(int low, int high, const std::function<int(T, T)>& compare) {
        if (low < high) {
            int pivot_idx = Partition(low, high, compare);
            Quicksort(low, pivot_idx - 1, compare);
            Quicksort(pivot_idx + 1, high, compare);
        }
    }

    int Partition(int low, int high, const std::function<int(T, T)>& compare) {
        T pivot = data[high];
        int i = (low - 1);

        for (int j = low; j <= high - 1; j++) {
            if (compare(data[j], pivot) < 0) {
                i++;
                Swap(i, j);
            }
        }
        Swap(i + 1, high);
        return (i + 1);
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
    }

    ~DynamicArray()
    {
        free(data);
    }

    inline void GrowCapacity(U64 toCapacity) 
    {
        capacity = toCapacity;
        data = (T*)realloc(data, capacity*sizeof(T));
        // Yes this is a memory leak, but we abort anyway.
        if(!data)
        {
            // When malloc fails the program is terminated if you ask me. So
            // this will result in a crash and thats fine.
            std::cerr << "No memory allocated in GrowCapacity" << std::endl;
            std::abort();
        }
    }

    inline T* IncreaseSize(int toSize)
    {
        if(toSize > capacity)
        {
            U64 new_capacity = toSize > 1000 ? (toSize + toSize/2) : (toSize * 2);
            GrowCapacity(new_capacity);
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

