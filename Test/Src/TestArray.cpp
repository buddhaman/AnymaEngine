#include "../test.h"
#include "Array.h"

extern MemoryArena* arena;

TEST_CASE("Array PushBack and indexing")
{
    Array<int> arr = CreateArray<int>(10);

    arr.PushBack(5);
    arr.PushBack(10);
    arr.PushBack(15);

    REQUIRE(arr.size == 3);
    REQUIRE(arr[0] == 5);
    REQUIRE(arr[1] == 10);
    REQUIRE(arr[2] == 15);
    REQUIRE(arr.At(0) == 5);

    DestroyArray(arr);
}

TEST_CASE("Array Fill and Clear")
{
    Array<int> arr = CreateArray<int>(5);

    arr.PushBack(1);
    arr.PushBack(2);
    REQUIRE(arr.size == 2);

    arr.Fill();
    REQUIRE(arr.size == 5);
    REQUIRE(arr.capacity == 5);

    arr.Clear();
    REQUIRE(arr.size == 0);

    DestroyArray(arr);
}

TEST_CASE("Array FillAndSetValue")
{
    Array<int> arr = CreateArray<int>(5);

    arr.FillAndSetValue(42);

    REQUIRE(arr.size == 5);
    for(int i = 0; i < arr.size; i++)
    {
        REQUIRE(arr[i] == 0x2A2A2A2A); // 42 repeated in each byte
    }

    DestroyArray(arr);
}

TEST_CASE("Array Swap")
{
    Array<int> arr = CreateArray<int>(5);
    arr.PushBack(10);
    arr.PushBack(20);
    arr.PushBack(30);

    arr.Swap(0, 2);

    REQUIRE(arr[0] == 30);
    REQUIRE(arr[2] == 10);
    REQUIRE(arr[1] == 20);

    DestroyArray(arr);
}

TEST_CASE("Array IsFull and Last")
{
    Array<int> arr = CreateArray<int>(3);

    REQUIRE(arr.IsFull() == false);

    arr.PushBack(1);
    arr.PushBack(2);
    arr.PushBack(3);

    REQUIRE(arr.IsFull() == true);
    REQUIRE(arr.Last() == 3);

    DestroyArray(arr);
}

TEST_CASE("Array View")
{
    Array<int> arr = CreateArray<int>(10);
    for(int i = 0; i < 10; i++)
    {
        arr.PushBack(i * 10);
    }

    Array<int> view = arr.View(2, 4);

    REQUIRE(view.capacity == 4);
    REQUIRE(view[0] == 20);
    REQUIRE(view[1] == 30);
    REQUIRE(view[2] == 40);
    REQUIRE(view[3] == 50);

    DestroyArray(arr);
}

TEST_CASE("Array Apply and ApplyIndexed")
{
    Array<int> arr = CreateArray<int>(5);
    arr.PushBack(1);
    arr.PushBack(2);
    arr.PushBack(3);

    arr.Apply([](int& x) { x *= 2; });

    REQUIRE(arr[0] == 2);
    REQUIRE(arr[1] == 4);
    REQUIRE(arr[2] == 6);

    arr.ApplyIndexed([](int idx, int& x) { x = idx * 10; });

    REQUIRE(arr[0] == 0);
    REQUIRE(arr[1] == 10);
    REQUIRE(arr[2] == 20);

    DestroyArray(arr);
}

TEST_CASE("Array Shift positive")
{
    Array<int> arr = CreateArray<int>(5);
    arr.PushBack(1);
    arr.PushBack(2);
    arr.PushBack(3);
    arr.PushBack(4);
    arr.PushBack(5);

    arr.Shift(2, 0);

    REQUIRE(arr[0] == 0);
    REQUIRE(arr[1] == 0);
    REQUIRE(arr[2] == 1);
    REQUIRE(arr[3] == 2);
    REQUIRE(arr[4] == 3);

    DestroyArray(arr);
}

TEST_CASE("Array Shift negative")
{
    Array<int> arr = CreateArray<int>(5);
    arr.PushBack(1);
    arr.PushBack(2);
    arr.PushBack(3);
    arr.PushBack(4);
    arr.PushBack(5);

    arr.Shift(-2, 0);

    REQUIRE(arr[0] == 3);
    REQUIRE(arr[1] == 4);
    REQUIRE(arr[2] == 5);
    REQUIRE(arr[3] == 0);
    REQUIRE(arr[4] == 0);

    DestroyArray(arr);
}

TEST_CASE("Array RemoveIndexUnordered")
{
    Array<int> arr = CreateArray<int>(5);
    arr.PushBack(10);
    arr.PushBack(20);
    arr.PushBack(30);
    arr.PushBack(40);

    arr.RemoveIndexUnordered(1);

    REQUIRE(arr.size == 3);
    REQUIRE(arr[0] == 10);
    REQUIRE(arr[1] == 40); // Last element moved here
    REQUIRE(arr[2] == 30);

    DestroyArray(arr);
}

TEST_CASE("Array RemoveIndex")
{
    Array<int> arr = CreateArray<int>(5);
    arr.PushBack(10);
    arr.PushBack(20);
    arr.PushBack(30);
    arr.PushBack(40);

    arr.RemoveIndex(1);

    REQUIRE(arr.size == 3);
    REQUIRE(arr[0] == 10);
    REQUIRE(arr[1] == 30); // Elements shifted
    REQUIRE(arr[2] == 40);

    DestroyArray(arr);
}

TEST_CASE("Array RemoveRange")
{
    Array<int> arr = CreateArray<int>(10);
    for(int i = 0; i < 10; i++)
    {
        arr.PushBack(i);
    }

    arr.RemoveRange(2, 5); // Remove indices 2,3,4,5 (inclusive)

    REQUIRE(arr.size == 6);
    REQUIRE(arr[0] == 0);
    REQUIRE(arr[1] == 1);
    REQUIRE(arr[2] == 6);
    REQUIRE(arr[3] == 7);
    REQUIRE(arr[4] == 8);
    REQUIRE(arr[5] == 9);

    DestroyArray(arr);
}

TEST_CASE("Array IndexOf")
{
    Array<int> arr = CreateArray<int>(5);
    arr.PushBack(10);
    arr.PushBack(20);
    arr.PushBack(30);

    REQUIRE(arr.IndexOf(20) == 1);
    REQUIRE(arr.IndexOf(30) == 2);
    REQUIRE(arr.IndexOf(99) == -1);

    DestroyArray(arr);
}

TEST_CASE("Array RemoveUnordered")
{
    Array<int> arr = CreateArray<int>(5);
    arr.PushBack(10);
    arr.PushBack(20);
    arr.PushBack(30);
    arr.PushBack(40);

    int removed_idx = arr.RemoveUnordered(20);

    REQUIRE(removed_idx == 1);
    REQUIRE(arr.size == 3);
    REQUIRE(arr.IndexOf(20) == -1);

    DestroyArray(arr);
}

TEST_CASE("Array MinElement and MaxElement")
{
    Array<int> arr = CreateArray<int>(10);
    arr.PushBack(50);
    arr.PushBack(10);
    arr.PushBack(90);
    arr.PushBack(30);
    arr.PushBack(70);

    REQUIRE(arr.MinElement() == 10);
    REQUIRE(arr.MaxElement() == 90);

    DestroyArray(arr);
}

TEST_CASE("Array Sort")
{
    Array<int> arr = CreateArray<int>(10);
    arr.PushBack(50);
    arr.PushBack(10);
    arr.PushBack(90);
    arr.PushBack(30);
    arr.PushBack(70);

    arr.Sort([](int a, int b) { return a - b; });

    REQUIRE(arr[0] == 10);
    REQUIRE(arr[1] == 30);
    REQUIRE(arr[2] == 50);
    REQUIRE(arr[3] == 70);
    REQUIRE(arr[4] == 90);

    DestroyArray(arr);
}

TEST_CASE("Array Iterator")
{
    Array<int> arr = CreateArray<int>(5);
    arr.PushBack(1);
    arr.PushBack(2);
    arr.PushBack(3);

    int sum = 0;
    for(int val : arr)
    {
        sum += val;
    }

    REQUIRE(sum == 6);

    DestroyArray(arr);
}

TEST_CASE("DynamicArray auto-growth")
{
    DynamicArray<int> arr;

    REQUIRE(arr.size == 0);
    REQUIRE(arr.capacity == 0);

    for(int i = 0; i < 100; i++)
    {
        arr.PushBack(i);
    }

    REQUIRE(arr.size == 100);
    REQUIRE(arr.capacity >= 100);

    for(int i = 0; i < 100; i++)
    {
        REQUIRE(arr[i] == i);
    }
}

TEST_CASE("DynamicArray initial capacity")
{
    DynamicArray<int> arr(50);

    REQUIRE(arr.capacity == 50);
    REQUIRE(arr.size == 0);

    for(int i = 0; i < 50; i++)
    {
        arr.PushBack(i * 2);
    }

    REQUIRE(arr.size == 50);
    REQUIRE(arr[25] == 50);
}

TEST_CASE("DynamicArray IncreaseSize")
{
    DynamicArray<int> arr(10);

    arr.IncreaseSize(5);
    REQUIRE(arr.size == 5);
    REQUIRE(arr.capacity == 10);

    arr.IncreaseSize(20);
    REQUIRE(arr.size == 20);
    REQUIRE(arr.capacity >= 20);
}

TEST_CASE("Arena-allocated Array")
{
    Array<int> arr = CreateArray<int>(arena, 10);

    arr.PushBack(1);
    arr.PushBack(2);
    arr.PushBack(3);

    REQUIRE(arr.size == 3);
    REQUIRE(arr.capacity == 10);
    REQUIRE(arr[0] == 1);
    REQUIRE(arr[1] == 2);
    REQUIRE(arr[2] == 3);
}

TEST_CASE("CopyArray")
{
    Array<int> original = CreateArray<int>(arena, 5);
    original.PushBack(10);
    original.PushBack(20);
    original.PushBack(30);

    Array<int> copy = CopyArray(arena, &original);

    REQUIRE(copy.size == 3);
    REQUIRE(copy.capacity == 5);
    REQUIRE(copy[0] == 10);
    REQUIRE(copy[1] == 20);
    REQUIRE(copy[2] == 30);

    // Verify they're separate
    copy[0] = 999;
    REQUIRE(original[0] == 10);
}
