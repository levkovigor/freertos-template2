/**
 * @file testPool.h
 *
 * @date 27.12.2019
 * @author R. Mueller
 */
#include <stdint.h>

#ifndef TEST_TESTINTERFACES_TESTPOOL_H_
#define TEST_TESTINTERFACES_TESTPOOL_H_

namespace TestInit{
struct TestIdStruct { //!< [EXPORT] : [DATA]
	uint32_t sid;
	struct ParameterList {
		uint32_t testBoolId;
		uint32_t testUint8Id;
		uint32_t testUint16Id;
		uint32_t testUint32Id;
		uint32_t testFloatVectorId;
	};
	ParameterList p;
	uint8_t numberOfParameters = sizeof(p) / 4;
	float collectionInterval = 5;
	bool isDiagnosticsPacket = false;
	bool enablePacket = false;
};

/**
 * Initiate GPS pool structs with datapool IDs for HK service
 * @param gps0
 * @param gps1
 */
void TestStructInit(TestIdStruct * test);
}


#endif /* TEST_TESTINTERFACES_TESTPOOL_H_ */
