#include <catch2/catch_test_macros.hpp>

#include <etl/multimap.h>
#include <cstring>

TEST_CASE( "ETL Map Test", "[etl]" ) {
    REQUIRE(std::strcmp("math", "math") == 0);
}
