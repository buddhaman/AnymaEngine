#include <iostream>

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>
#include <catch2/catch_approx.hpp>

#include "../../src/linalg.h"
#include "../../src/Memory.cpp"

MemoryArena* arena;

using namespace Catch;

TEST_CASE("Vector operations", "[VecR32]") 
{
    SECTION("Sum of vector elements") 
    {
        R32 data[] = {1.0, 2.0, 3.0};
        VecR32 vec = VecR32Create(3, data);
        REQUIRE(vec.Sum() == 6.0);
    }

    SECTION("Average of vector elements") 
    {
        R32 data[] = {2.0, 4.0, 6.0};
        VecR32 vec = VecR32Create(3, data);
        REQUIRE(vec.Avg() == 4.0);
    }

    SECTION("Vector addition") 
    {
        R32 dataA[] = {1.0, 2.0, 3.0};
        R32 dataB[] = {3.0, 2.0, 1.0};
        R32 resultData[3] = {0.0, 0.0, 0.0};
        VecR32 a = VecR32Create(3, dataA);
        VecR32 b = VecR32Create(3, dataB);
        VecR32 result = VecR32Create(3, resultData);
        
        VecR32Add(result, a, b);
        REQUIRE(result.v[0] == 4.0);
        REQUIRE(result.v[1] == 4.0);
        REQUIRE(result.v[2] == 4.0);
    }
}

TEST_CASE("Allocation")
{
    VecR32 a = VecR32Create(arena, 12);
    a.Apply([](int i, R32 x) -> R32 {return i;});

    VecR32 b = VecR32Create(arena, 12);
    b.Apply([](R32 x) -> R32 {return 2;});

    VecR32 result = VecR32Create(arena, 12);
    VecR32Mul(result, a, b);

    REQUIRE(result[10]==Approx(20.0f));
}

TEST_CASE("Random number generation")
{
    VecR32 a = VecR32Create(arena, 120);

    R32 target_avg = 14.0f;
    R32 target_dev = 1.0f;
    a.SetNormal(target_avg, target_dev);

    REQUIRE(a.Avg() == Approx(target_avg).margin(0.1f));
    REQUIRE(a.StdDev() == Approx(target_dev).margin(0.1f));
}

TEST_CASE("Matrix-vector multiplication", "[MatR32]") 
{
    SECTION("Matrix-vector multiplication") 
    {
        R32 matData[] = {1, 2, 3, 4, 5, 6};
        R32 vecData[] = {1, 2, 3};
        R32 resultData[2] = {0.0, 0.0};
        MatR32 mat = MatR32Create(3, 2, matData); // 2x3 matrix
        VecR32 vec = VecR32Create(3, vecData);
        VecR32 result = VecR32Create(2, resultData);
        
        MatR32VecMul(result, mat, vec);
        REQUIRE(result.v[0] == Approx(14.0f)); // Row 1: 1*1 + 2*2 + 3*3
        REQUIRE(result.v[1] == Approx(32.0f)); // Row 2: 4*1 + 5*2 + 6*3
    }
}

int main(int argc, char** argv)
{
    arena = CreateMemoryArena(MegaBytes(16));
    int result = Session().run(argc, argv);
    return result;
}