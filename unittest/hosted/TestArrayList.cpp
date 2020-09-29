#include <CatchDefinitions.h>
#include <catch.hpp>
#include <fsfw/container/ArrayList.h>

/**
 * @brief 	Array List test
 */
TEST_CASE("Array List" , "[ArrayListTest]") {
	//perform set-up here
	ArrayList<uint16_t> list = ArrayList<uint16_t>(3);
	list.insert(1);
	list.insert(2);
	ArrayList<uint16_t>::Iterator start = list.begin();
	ArrayList<uint16_t>::Iterator end = list.end();
	SECTION("SimpleTest") {
		// set-up is run for each section
		bool test = (start != end);
		CHECK(test);
		while(start != list.end()) {
			start ++;
		}
		CHECK(start == end);

		ReturnValue_t result = list.insert(3);
		CHECK(result == retval::CATCH_OK);
		CHECK(list.insert(6) == (uint16_t) ArrayList<uint16_t>::FULL);
	}
	// perform tear-down here
}
