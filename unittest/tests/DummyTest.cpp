#include <catch2/catch_test_macros.hpp>

#include <cstring>

TEST_CASE( "Dummy Test", "[dummy]" ) {
    REQUIRE(std::strcmp("math", "math") == 0);
}
