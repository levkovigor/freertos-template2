#include "RingBufferTest.h"
#include <fsfw/container/SimpleRingBuffer.h>

#include <CatchDefinitions.h>
#include <catch.hpp>
#include <cstring>

TEST_CASE("Ring Buffer Test" , "[RingBufferTest]") {
	uint8_t testData[13]= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
	uint8_t readBuffer[10];
	SimpleRingBuffer ringBuffer(10, false, 5);

	SECTION("Simple Test") {
		REQUIRE(ringBuffer.availableWriteSpace() == 9);
		REQUIRE(ringBuffer.writeData(testData, 9) == retval::CATCH_OK);
		REQUIRE(ringBuffer.writeData(testData, 3) == retval::CATCH_FAILED);
		REQUIRE(ringBuffer.readData(readBuffer, 5, true) == retval::CATCH_OK);
		for(uint8_t i = 0; i< 5; i++) {
			CHECK(readBuffer[i] == i);
		}
		REQUIRE(ringBuffer.availableWriteSpace() == 5);
		ringBuffer.clear();
		REQUIRE(ringBuffer.availableWriteSpace() == 9);
	}

	SECTION("Get Free Element Test") {
		REQUIRE(ringBuffer.availableWriteSpace() == 9);
		REQUIRE(ringBuffer.writeData(testData, 8) == retval::CATCH_OK);
		REQUIRE(ringBuffer.availableWriteSpace() == 1);
		REQUIRE(ringBuffer.readData(readBuffer, 8, true) == retval::CATCH_OK);
		REQUIRE(ringBuffer.availableWriteSpace() == 9);

		uint8_t *testPtr = nullptr;
		REQUIRE(ringBuffer.writeTillWrap() == 2);
		// too many excess bytes.
		REQUIRE(ringBuffer.getFreeElement(&testPtr, 8) ==  retval::CATCH_FAILED);
		REQUIRE(ringBuffer.getFreeElement(&testPtr, 5) ==  retval::CATCH_OK);
		REQUIRE(ringBuffer.getExcessBytes() == 3);
		std::memcpy(testPtr, testData, 5);
		ringBuffer.confirmBytesWritten(5);
		REQUIRE(ringBuffer.getAvailableReadData() == 5);
		ringBuffer.readData(readBuffer, 5, true);
		for(uint8_t i = 0; i< 5; i++) {
			CHECK(readBuffer[i] == i);
		}
	}
}
