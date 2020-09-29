#include <CatchDefinitions.h>
#include <catch.hpp>
#include <fsfw/container/ArrayList.h>

/**
 * @brief 	Array List test
 */
TEST_CASE("Array List" , "[ArrayListTest]") {
	//perform set-up here
	ArrayList<uint16_t> list = ArrayList<uint16_t>(20);
	SECTION("SimpleTest") {
		// set-up is run for each section

	}
	// perform tear-down here
}
