#include <iostream>

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>
#include <catch2/catch_approx.hpp>

#include "../../Src/String.h"

TEST_CASE("Test string basics")
{
    String8 lit = CopyFromLiteral("hellow");
    std::cout << lit.data << "\n";

    String8 lit1 = CopyFromLiteral("hellow");
    String8 lit2 = CopyFromLiteral(" T is me");
    String8 concat = Concat(lit1, lit2);
    std::cout << concat.data << "\n";
}
