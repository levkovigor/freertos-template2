#include <catch2/catch_test_macros.hpp>

#include <cstring>

// Catch2 documentation:
// https://github.com/catchorg/Catch2/blob/devel/docs/assertions.md
TEST_CASE( "Dummy Test", "[dummy]" ) {
    REQUIRE(std::strcmp("math", "math") == 0);
}
