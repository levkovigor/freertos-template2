/**
 * @file testPool.cpp
 *
 * @date 27.12.2019
 * @author R. Mueller
 */

#include <config/cdatapool/dataPoolInit.h>
#include <config/cdatapool/testPool.h>
#include <config/hk/sid.h>

void TestInit::TestStructInit(TestIdStruct * test) {
	test->p.testBoolId = datapool::TEST_BOOLEAN;
	test->p.testUint8Id = datapool::TEST_UINT8;
	test->p.testUint16Id = datapool::TEST_UINT16;
	test->p.testUint32Id = datapool::TEST_UINT32;
	test->p.testFloatVectorId = datapool::TEST_FLOAT_VECTOR;

	float TEST_COLLECTION_INTERVAL = 7;
	test->sid = sid::TEST;
	test->collectionInterval = TEST_COLLECTION_INTERVAL;
	test->isDiagnosticsPacket = false;
}


