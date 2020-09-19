#include "HammingTest.h"

#include <CatchDefinitions.h>
#include <catch.hpp>

extern "C" {
#include <at91/utility/hamming.h>
}


TEST_CASE("HammingTest" , "[HammingTest]") {
	//perform set-up here
	CHECK(testVariable == 1);
    uint8_t test[256];
    for(auto idx = 0; idx < 128; idx++) {
        test[idx] = idx;
    }
    for(auto idx = 0; idx < 128; idx++) {
        test[idx + 128] = 128 - idx ;
    }
    uint8_t hamming[3];
    Hamming_Compute256x(test, 256, hamming);

    sif::info << "Hamming code: " << (int) hamming[0] << (int) hamming[1]
             << (int) hamming[2] << std::endl;
	SECTION("TestSection") {
		// set-up is run for each section
		REQUIRE(hamming[0] == 0x5A);
		REQUIRE(hamming[0] == 0xAA);
		REQUIRE(hamming[0] == 0x57);
	}
	// todo: add all the tests which will also be used to verify python code.

}
