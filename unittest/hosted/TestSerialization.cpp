#include <fsfw/serialize/SerializeAdapter.h>
#include <unittest/hosted/TestSerialization.h>

#include "catch.hpp"
#include "CatchDefinitions.h"

#include <array>


TEST_CASE( "Serialization size tests", "[TestSerialization]") {
	//REQUIRE(unitTestClass.test_autoserialization() == 0);
	REQUIRE(SerializeAdapter::getSerializedSize(&test_value_bool) ==
			sizeof(test_value_bool));
	REQUIRE(SerializeAdapter::getSerializedSize(&tv_uint8) ==
			sizeof(tv_uint8));
	REQUIRE(SerializeAdapter::getSerializedSize(&tv_uint16) ==
			sizeof(tv_uint16));
	REQUIRE(SerializeAdapter::getSerializedSize(&tv_uint32 ) ==
			sizeof(tv_uint32));
	REQUIRE(SerializeAdapter::getSerializedSize(&tv_uint64) ==
			sizeof(tv_uint64));
	REQUIRE(SerializeAdapter::getSerializedSize(&tv_int8) ==
			sizeof(tv_int8));
	REQUIRE(SerializeAdapter::getSerializedSize(&tv_int16) ==
			sizeof(tv_int16));
	REQUIRE(SerializeAdapter::getSerializedSize(&tv_int32) ==
			sizeof(tv_int32));
	REQUIRE(SerializeAdapter::getSerializedSize(&tv_float) ==
			sizeof(tv_float));
	REQUIRE(SerializeAdapter::getSerializedSize(&tv_sfloat) ==
			sizeof(tv_sfloat ));
	REQUIRE(SerializeAdapter::getSerializedSize(&tv_double) ==
			sizeof(tv_double));
	REQUIRE(SerializeAdapter::getSerializedSize(&tv_sdouble) ==
			sizeof(tv_sdouble));
}


TEST_CASE("Auto Serialize Adapter testing", "[single-file]") {
	size_t serialized_size = 0;
	uint8_t * p_array = test_array.data();

	SECTION("Serializing...") {
		SerializeAdapter::serialize(&test_value_bool, &p_array,
				&serialized_size, test_array.size(), SerializeIF::Endianness::MACHINE);
		SerializeAdapter::serialize(&tv_uint8, &p_array,
				&serialized_size, test_array.size(), SerializeIF::Endianness::MACHINE);
		SerializeAdapter::serialize(&tv_uint16, &p_array,
				&serialized_size, test_array.size(), SerializeIF::Endianness::MACHINE);
		SerializeAdapter::serialize(&tv_uint32, &p_array,
				&serialized_size, test_array.size(), SerializeIF::Endianness::MACHINE);
		SerializeAdapter::serialize(&tv_int8, &p_array,
				&serialized_size, test_array.size(), SerializeIF::Endianness::MACHINE);
		SerializeAdapter::serialize(&tv_int16, &p_array,
				&serialized_size, test_array.size(), SerializeIF::Endianness::MACHINE);
		SerializeAdapter::serialize(&tv_int32, &p_array,
				&serialized_size, test_array.size(), SerializeIF::Endianness::MACHINE);
		SerializeAdapter::serialize(&tv_uint64, &p_array,
				&serialized_size, test_array.size(), SerializeIF::Endianness::MACHINE);
		SerializeAdapter::serialize(&tv_float, &p_array,
				&serialized_size, test_array.size(), SerializeIF::Endianness::MACHINE);
		SerializeAdapter::serialize(&tv_double, &p_array,
				&serialized_size, test_array.size(), SerializeIF::Endianness::MACHINE);
		SerializeAdapter::serialize(&tv_sfloat, &p_array,
				&serialized_size, test_array.size(), SerializeIF::Endianness::MACHINE);
		SerializeAdapter::serialize(&tv_sdouble, &p_array,
				&serialized_size, test_array.size(), SerializeIF::Endianness::MACHINE);
		REQUIRE (serialized_size == 47);
	}

	SECTION("Deserializing") {
		p_array = test_array.data();
		size_t remaining_size = serialized_size;
		SerializeAdapter::deSerialize(&test_value_bool,
				const_cast<const uint8_t**>(&p_array), &remaining_size, SerializeIF::Endianness::MACHINE);
		SerializeAdapter::deSerialize(&tv_uint8,
				const_cast<const uint8_t**>(&p_array), &remaining_size, SerializeIF::Endianness::MACHINE);
		SerializeAdapter::deSerialize(&tv_uint16,
				const_cast<const uint8_t**>(&p_array), &remaining_size, SerializeIF::Endianness::MACHINE);
		SerializeAdapter::deSerialize(&tv_uint32,
				const_cast<const uint8_t**>(&p_array), &remaining_size, SerializeIF::Endianness::MACHINE);
		SerializeAdapter::deSerialize(&tv_int8,
				const_cast<const uint8_t**>(&p_array), &remaining_size, SerializeIF::Endianness::MACHINE);
		SerializeAdapter::deSerialize(&tv_int16,
				const_cast<const uint8_t**>(&p_array), &remaining_size, SerializeIF::Endianness::MACHINE);
		SerializeAdapter::deSerialize(&tv_int32,
				const_cast<const uint8_t**>(&p_array), &remaining_size, SerializeIF::Endianness::MACHINE);
		SerializeAdapter::deSerialize(&tv_uint64,
				const_cast<const uint8_t**>(&p_array), &remaining_size, SerializeIF::Endianness::MACHINE);
		SerializeAdapter::deSerialize(&tv_float,
				const_cast<const uint8_t**>(&p_array), &remaining_size, SerializeIF::Endianness::MACHINE);
		SerializeAdapter::deSerialize(&tv_double,
				const_cast<const uint8_t**>(&p_array), &remaining_size, SerializeIF::Endianness::MACHINE);
		SerializeAdapter::deSerialize(&tv_sfloat,
				const_cast<const uint8_t**>(&p_array), &remaining_size, SerializeIF::Endianness::MACHINE);
		SerializeAdapter::deSerialize(&tv_sdouble,
				const_cast<const uint8_t**>(&p_array), &remaining_size, SerializeIF::Endianness::MACHINE);

		REQUIRE(test_value_bool == true);
		REQUIRE(tv_uint8 == 5);
		REQUIRE(tv_uint16 == 283);
		REQUIRE(tv_uint32 == 929221);
		REQUIRE(tv_uint64 == 2929329429);
		REQUIRE(tv_int8 == -16);
		REQUIRE(tv_int16 == -829);
		REQUIRE(tv_int32 == -2312);

		REQUIRE(tv_float == Approx(8.214921));
		REQUIRE(tv_double == Approx(9.2132142141e8));
		REQUIRE(tv_sfloat == Approx(-922.2321321));
		REQUIRE(tv_sdouble == Approx(-2.2421e19));
	}
}


