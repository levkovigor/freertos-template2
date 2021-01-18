#ifndef TEST_TESTTASKS_TESTARCHIVE_H_
#define TEST_TESTTASKS_TESTARCHIVE_H_
#include <cstdint>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>
#include <fsfw/serialize/SerialFixedArrayListAdapter.h>
#include <fsfw/serialize/SerialLinkedListAdapter.h>

namespace tarchive {

void mapOrderTesting();

void testIntShifting();
void shiftIntoInteger(uint32_t & valueToSet, uint8_t test1,
		uint16_t test2);
#if __cplusplus > 201703L
void shiftOutOfInteger(uint32_t value, std::byte & test1,
        uint16_t & test2);
#else
void shiftOutOfInteger(uint32_t value, uint8_t & test1,
		uint16_t & test2);
#endif
ReturnValue_t performSerialFixedArrayTesting();

void testStructContent();
#if __cplusplus > 201703L
struct TestStruct {
	bool testBool = false;
	bool * testBoolPointer = nullptr;
};
#else
struct TestStruct {
	bool testBool;
	bool * testBoolPointer;
};
#endif

void passingStruct(TestStruct testStruct);

void performEndiannessTesting();
}


//class TestSerialFixedArrayListAdapter: public SerialLinkedListAdapter<SerializeIF> {
//public:
//    TestSerialFixedArrayListAdapter(uint8_t testHeader_,
//            uint8_t * data1_, uint8_t dataSize1_,
//            uint16_t * data2_, uint8_t dataSize2_,
//            uint16_t * data3_, uint16_t dataSize3_,
//            uint16_t testTail_):
//                testHeader(testHeader_),
//                data1(data1_,dataSize1_,true),
//                data2(data2_,dataSize2_,true),
//                data3(data3_,dataSize3_,false),
//                testTail(testTail_){
//        setLinks();
//    }
//    SerializeElement<uint8_t> testHeader;
//    SerializeElement<SerialFixedArrayListAdapter<uint8_t,128,uint8_t>> data1;
//    SerializeElement<uint8_t> divider = 5;
//    SerializeElement<SerialFixedArrayListAdapter<uint16_t,128,uint8_t>> data2;
//    SerializeElement<uint8_t> divider2 = 5;
//    SerializeElement<SerialFixedArrayListAdapter<uint16_t,128,uint16_t>> data3;
//    SerializeElement<uint16_t> testTail;
//
//    void setLinks() {
//        setStart(&testHeader);
//        testHeader.setNext(&data1);
//        //data1.setNext(&testTail);
//        //testTail.setEnd();
//        data1.setNext(&divider);
//        divider.setNext(&data2);
//        data2.setNext(&divider2);
//        divider2.setNext(&data3);
//        data3.setNext(&testTail);
//    }
//};

#endif /* TEST_TESTTASKS_TESTARCHIVE_H_ */
