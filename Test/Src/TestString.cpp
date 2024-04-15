#include <iostream>

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>
#include <catch2/catch_approx.hpp>

#include "../../src/String.h"

TEST_CASE("Test string basics")
{
    String8 lit = CopyFromLiteral("hellow");
    std::cout << lit.data << "\n";
}
