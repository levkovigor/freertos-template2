#include <catch2/catch_test_macros.hpp>
#include <mission/memory/tmstore/EtlIMultiMapWrapper.h>

#include <etl/multimap.h>
#include <cstring>

#include <iostream>

TEST_CASE( "ETL Map Test", "[etl]" ) {
    REQUIRE(std::strcmp("math", "math") == 0);
    etl::multimap<int, int, 3> testMap;
    EtlIMultiMapWrapper<int, int> wrapper(&testMap);
    CHECK(wrapper.emplace(0, 5) == (int) HasReturnvaluesIF::RETURN_OK);
    CHECK(wrapper.emplace(0, 32) == (int) HasReturnvaluesIF::RETURN_OK);
    CHECK(wrapper.emplace(0, 932) == (int) HasReturnvaluesIF::RETURN_OK);
    CHECK(wrapper.emplace(0, 932) == (int) HasReturnvaluesIF::RETURN_FAILED);

    MultiMapGetReturn<int, int> getResult = wrapper.get(0);
    for(auto startIter = getResult.begin; startIter != getResult.end; startIter++) {
        auto value = *startIter;
        // std::cout << "key: " << value.first << ", value: " <<
        //      value.second << std::endl;
    }

}
