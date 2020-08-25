//#include <fsfw/datapoollocal/LocalPoolVariable.h>
//#include <fsfw/datapoollocal/LocalPoolVector.h>
//#include <unittest/hosted/LocalDataPoolTest.h>
//#include "catch.hpp"
//
//TEST_CASE("Local Data Pool Test" , "[LocalDataPoolTest]") {
//	//perform set-up here
//	object_id_t dummyTestId = 0x1234321;
//	LocalDataPoolDummy localDataPoolClass(dummyTestId);
//	auto result = objectManager->insert(dummyTestId, &localDataPoolClass);
//	REQUIRE(result == retval::CATCH_OK);
//	SECTION("Read variables") {
//		lp_bool_t boolVar(
//				static_cast<int>(LocalDataPoolDummy::PoolIds::POOL_VAR_1),
//				&localDataPoolClass
//		);
//
//		boolVar.read(MutexIF::POLLING);
//		CHECK(boolVar.value == false);
//		boolVar.value = true;
//		boolVar.commit(MutexIF::POLLING);
//		boolVar.read(MutexIF::POLLING);
//		CHECK(boolVar.value == true);
//
//		lp_float_t floatVar(
//				static_cast<int>(LocalDataPoolDummy::PoolIds::POOL_VAR_2),
//				&localDataPoolClass
//		);
//		floatVar.read(MutexIF::POLLING);
//		CHECK(floatVar.value == Approx(2.5243));
//		CHECK(not floatVar.isValid());
//
//		floatVar.setValid(true);
//		floatVar.value = -5;
//		floatVar.commit();
//
//		floatVar.read();
//		CHECK(floatVar.value == Approx(-5));
//		CHECK(floatVar.isValid());
//
//		lp_vec_t<uint16_t, 3> uint16Vec(
//				static_cast<int>(LocalDataPoolDummy::PoolIds::POOL_VEC_1),
//				&localDataPoolClass
//		);
//
//		uint16Vec.read();
//		CHECK(uint16Vec[0]== 0);
//		uint16Vec[0] = 10;
//		CHECK(uint16Vec[0] == 10);
//		CHECK(uint16Vec.value[1] == 0);
//		CHECK(uint16Vec.value[2] == 2);
//		CHECK(uint16Vec.isValid() == false);
//		// set-up is run for each section
//
//	}
//
//	SECTION("Test Data Set") {
//		/* I am using the classes own pool var and dataset instances,
//		 * these are public, so I get read and write access.
//		 * If this is not desired, the variables
//		 * should be made privateand the user should declare own pool variables/
//		 * datasets, which only have the desired permissions. */
//		sid_t testSid;
//		testSid.objectId = dummyTestId;
//		testSid.ownerSetId = 0;
//		LocalDataSet* testDataSet =
//				dynamic_cast<LocalDataSet*>(
//				localDataPoolClass.getDataSetHandle(testSid));
//		REQUIRE(testDataSet != nullptr);
//		REQUIRE(testDataSet->read(MutexIF::POLLING) == retval::CATCH_OK);
//		CHECK(testDataSet->read(MutexIF::POLLING) ==
//				static_cast<int>(DataSetIF::SET_WAS_ALREADY_READ));
//		CHECK(localDataPoolClass.ownerBoolVar.value == false);
//		CHECK(not localDataPoolClass.ownerBoolVar.isValid());
//		CHECK(localDataPoolClass.ownerFloatVar.value == Approx(2.5243));
//		CHECK(localDataPoolClass.ownerUint16Vec.value[0] == 0);
//		CHECK(localDataPoolClass.ownerUint16Vec.value[1] == 0);
//		CHECK(localDataPoolClass.ownerUint16Vec.value[2] == 2);
//
//		localDataPoolClass.ownerFloatVar.value = -5;
//		localDataPoolClass.ownerUint16Vec.value[0] = 10;
//		localDataPoolClass.ownerBoolVar.setValid(true);
//		REQUIRE(testDataSet->commit(MutexIF::POLLING) == retval::CATCH_OK);
//
//		REQUIRE(testDataSet->read(MutexIF::POLLING) == retval::CATCH_OK);
//		CHECK(localDataPoolClass.ownerFloatVar.value == Approx(-5));
//		CHECK(localDataPoolClass.ownerBoolVar.isValid());
//		CHECK(localDataPoolClass.ownerUint16Vec.value[0] == 10);
//		REQUIRE(testDataSet->commit(MutexIF::POLLING) == retval::CATCH_OK);
//	}
//	// perform tear-down here
//	objectManager->remove(dummyTestId);
//}
