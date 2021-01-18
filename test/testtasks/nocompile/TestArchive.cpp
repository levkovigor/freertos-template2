#include <test/testtasks/TestArchive.h>
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <test/testtasks/TestTask.h>

#include <map>
#include <unordered_map>
#include <bitset>

void tarchive::mapOrderTesting() {
	sif::info << "Using map:" << std::endl;
	std::map<uint16_t, std::string> testMap;
	testMap.emplace(0, "Hallo!");
	testMap.emplace(1, "Hallo 2!");
	testMap.emplace(2, "Hallo 3!");
	int counter = 1;
	for(auto testMapIter = testMap.begin();
			testMapIter != testMap.end(); testMapIter++) {
		sif::info << "Test Map entry at position "
			 << counter ++ <<": " << testMapIter->second << std::endl;
	}

	sif::info << "Using unordered map:" << std::endl;
	std::unordered_map<uint16_t, std::string> testMap2;
	testMap2.emplace(0, "Hallo!");
	testMap2.emplace(1, "Hallo 2!");
	testMap2.emplace(2, "Hallo 3!");
	counter = 1;
	for(auto testMapIter = testMap2.begin();
			testMapIter != testMap2.end(); testMapIter++) {
		sif::info << "Test Map entry at position "
				<< counter ++ <<": " << testMapIter->second << std::endl;
	}
}


void tarchive::testIntShifting() {
	uint16_t test = 0;
	uint8_t firstByte = 1;
	sif::info << "Setting first byte of uint16 to 1" << std::endl;
	test = (test & 0x00FF) + (firstByte << 8);
	sif::info << std::bitset<16>(test) << std::endl;
	sif::info << test << std::endl;

	firstByte = 2;
	sif::info << "Setting first byte of uint16 to 2" << std::endl;
	test = (test & 0x00FF) + (firstByte << 8);
	sif::info << std::bitset<16>(test) << std::endl;
	sif::info << test << std::endl;

	uint32_t testValue = 0;
	shiftIntoInteger(testValue, 5, 16);
	sif::info << "Test value after shifting 5 and 16 into test value: " << std::endl;
	sif::info <<  std::bitset<32>(testValue) << std::endl;
#if __cplusplus > 201703L
	std::byte byteOne;
#else
	uint8_t byteOne;
#endif
	uint16_t byteTwoAndThree;
	shiftOutOfInteger(testValue, byteOne, byteTwoAndThree);
	//info << "Test value one: " << std::to_integer<int>(byteOne) << std::endl;
	//info << "Test value two: " << byteTwoAndThree << std::endl;
}

void tarchive::shiftIntoInteger(uint32_t & valueToSet, uint8_t test1,
		uint16_t test2) {
	sif::info << "Init Test Value: " << valueToSet << std::endl;
	// Shift test1 into first byte to valueToSet
	valueToSet = (valueToSet & 0x00FFFFFF) + (test1 << 24);
	sif::info << "Init Test after shifting in first byte: " << valueToSet << std::endl;
	sif::info << "In bit form: " << std::bitset<32>(valueToSet) << std::endl;
	// Shift test2 into second and third byte of valueToSet
	valueToSet = (valueToSet & 0xFF0000FF) + (test2 << 8);
	sif::info << "Init Test after shifting in second and third byte: "
		 << valueToSet << std::endl;
	sif::info << "In bit form: " << std::bitset<32>(valueToSet) << std::endl;
}

#if __cplusplus > 201703L
void tarchive::shiftOutOfInteger(uint32_t value, std::byte & test1,
		uint16_t & test2) {
	// extract first byte
	test1 = static_cast<std::byte>((value >> 24) & 0xFF);
	test2 = (value >> 8) & 0xFFFF;
}
#else

void tarchive::shiftOutOfInteger(uint32_t value, uint8_t & test1,
		uint16_t & test2) {
	// extract first byte
	test1 = ((value >> 24) & 0xFF);
	test2 = (value >> 8) & 0xFFFF;
}
#endif

//ReturnValue_t tarchive::performSerialFixedArrayTesting() {
//	uint8_t header = 10;
//	uint8_t testBuffer1[3] = {1, 2, 3};
//	uint16_t testBuffer2[3];
//	testBuffer2[0] = 0x0001;
//	testBuffer2[1] = 0x0002;
//	testBuffer2[2] = 0x0003;
//	uint16_t tail = 10;
//	uint16_t testBuffer3[3];
//	testBuffer3[0] = 0x0001;
//	testBuffer3[1] = 0x0002;
//	testBuffer3[2] = 0x0003;
//	TestSerialFixedArrayListAdapter test(header,
//			reinterpret_cast<uint8_t *>(testBuffer1), 3,
//			reinterpret_cast<uint16_t *>(testBuffer2), 3,
//			reinterpret_cast<uint16_t *>(testBuffer3), 3, tail);
//	uint8_t testSerialBuffer[512];
//	uint8_t * pTestSerialBuffer = reinterpret_cast<uint8_t *>(&testSerialBuffer);
//	size_t size = 0;
//	test.serialize(&pTestSerialBuffer,&size,512,true);
//	sif::info << "Test Serialization, size is " << (int)test.getSerializedSize()
//			<< " bytes: [";
//	for(uint8_t count = 0;count < test.getSerializedSize();count ++) {
//		sif::info << std::dec << (int)testSerialBuffer[count] << std::dec <<  " ";
//	}
//	sif::info << "]" << std::endl;
//	return HasReturnvaluesIF::RETURN_OK;
//}



void tarchive::testStructContent() {
	bool testBool = true;
	bool testBool2 = false;

	TestStruct testStruct = {testBool, &testBool2};
	sif::info << "Test bool stored directly in struct: ";
	testStruct.testBool = true;
	sif::info << testBool << std::endl;
	testStruct.testBool = false;
	sif::info << testBool << std::endl;

	sif::info << "Test bool pointer stored in struct: " << std::endl;
	*testStruct.testBoolPointer = true;
	sif::info << testBool2 << std::endl;
	*testStruct.testBoolPointer = false;
	sif::info << testBool2 << std::endl;
	passingStruct(testStruct);
	sif::info << testBool2 << std::endl;
}

void tarchive::passingStruct(TestStruct testStruct) {
	*testStruct.testBoolPointer = true;
}

void tarchive::performEndiannessTesting() {
	uint32_t hardcoded_id_big_endian = 0x01010102;
	sif::info << "Hardcoded ID Target value: " << hardcoded_id_big_endian << std::endl;
	const uint8_t test_id_raw_from_ground[4] = {0x01, 0x01, 0x01, 0x02};
	const uint8_t* crappointer = static_cast<const uint8_t*>(test_id_raw_from_ground);
	uint32_t test_id_to_store = 0;
	size_t size = 4;
	SerializeAdapter::deSerialize(&test_id_to_store, &crappointer,&size,
	        SerializeIF::Endianness::BIG);
	//memcpy(&test_id_to_store, test_id_raw_from_ground, 4);
	sif::info << "Deserialized ID, unswapped: " << test_id_to_store << std::endl;
	size = 4;
	crappointer = test_id_raw_from_ground;
	SerializeAdapter::deSerialize(&test_id_to_store, &crappointer,&size,
	        SerializeIF::Endianness::BIG);
	//test_id_to_store = EndianSwapper::swap(test_id_to_store);
	sif::info << "Deserialized ID, swapped: " << test_id_to_store << std::endl;

	uint32_t hardcoded_id_to_serialize = 0x01010102;
	std::array<uint8_t, 4> test_buffer{ 0, 0, 0, 0};
	size = 4;
	uint8_t* p_buf = test_buffer.data();

	SerializeAdapter::serialize(
			&hardcoded_id_to_serialize, &p_buf , &size, 1024,
			SerializeIF::Endianness::BIG);
	sif::info << "Serialized hardcoded ID raw[0]: " << (int) test_buffer[0] << std::endl;
	sif::info << "Serialized hardcoded ID raw[3]: " <<(int) test_buffer[3] << std::endl;
	memcpy(&test_id_to_store, test_buffer.data(), 4);
	sif::info << "Deserialized after serialization, swapped serialization: " <<
			test_id_to_store << std::endl;
	p_buf = test_buffer.data();
	SerializeAdapter::serialize(
			&hardcoded_id_to_serialize, &p_buf , &size, 1024,
			SerializeIF::Endianness::MACHINE);
	sif::info << "Serialized hardcoded ID, swapped: " << (int) test_buffer[0] << std::endl;
	sif::info << "Serialized hardcoded ID, swapped: " <<(int) test_buffer[3] << std::endl;
	memcpy(&test_id_to_store, test_buffer.data(), 4);
	sif::info << "Deserialized (memcpy) after serialization, "
			"unswapped serialization: " << test_id_to_store << std::endl;
	p_buf = test_buffer.data();
	SerializeAdapter::deSerialize(&test_id_to_store,
			const_cast<const uint8_t**>(&p_buf), &size,
			SerializeIF::Endianness::MACHINE);
	sif::info << "Deserialized (Auto) after serialization, "
				"unswapped serialization: " << test_id_to_store << std::endl;

	float test_time = 0.4;
	uint8_t header = 2;
	uint16_t tail = 5;
	std::array<uint8_t, 7> test_array;
	p_buf = test_array.data();
	size_t ser_size = 0;
	SerializeAdapter::serialize(&header, &p_buf, &ser_size, 1024,
	        SerializeIF::Endianness::MACHINE);
	SerializeAdapter::serialize(&test_time, &p_buf, &ser_size, 1024,
	        SerializeIF::Endianness::MACHINE);
	SerializeAdapter::serialize(&tail, &p_buf, &ser_size, 1024,
	        SerializeIF::Endianness::MACHINE);
	p_buf = test_array.data();
	SerializeAdapter::deSerialize(&header,
			const_cast<const uint8_t**>(&p_buf),&ser_size,
			SerializeIF::Endianness::MACHINE);
	SerializeAdapter::deSerialize(&test_time,
				const_cast<const uint8_t**>(&p_buf),&ser_size,
				SerializeIF::Endianness::MACHINE);
	SerializeAdapter::deSerialize(&tail,
				const_cast<const uint8_t**>(&p_buf),&ser_size,
				SerializeIF::Endianness::MACHINE);
	sif::info << "Header: " << (int) header << std::endl;
	sif::info << "Test Time: " << test_time << std::endl;
	sif::info << "Tail:  " << tail << std::endl;
}
