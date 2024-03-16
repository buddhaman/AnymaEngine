#include <iostream>

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>

#include "../../src/linalg.h"

TEST_CASE("Addition")
{
    VecR32 coolVec;
}

int main(int argc, char** argv)
{
    int result = Catch::Session().run(argc, argv);
    return result;
}