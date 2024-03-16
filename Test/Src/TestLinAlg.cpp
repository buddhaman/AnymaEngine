#include <iostream>

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>

TEST_CASE("Addition")
{
    REQUIRE(1 + 2 == 3);
    REQUIRE(1 + 2 == 4);
}

int main(int argc, char** argv)
{
    int result = Catch::Session().run(argc, argv);
    return result;
}