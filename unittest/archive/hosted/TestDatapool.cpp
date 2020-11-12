#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/datapoolglob/GlobalDataSet.h>
#include <fsfw/datapoolglob/GlobalPoolVariable.h>
#include <fsfw/datapoolglob/GlobalPoolVector.h>

#include <catch.hpp>
#include <CatchDefinitions.h>
#include <fsfwconfig/cdatapool/dataPoolInit.h>

TEST_CASE( "Simple global datapool test", "[TestDataPool]") {
	GlobDataSet testDataSet;
	gp_uint8_t testUint8 = gp_uint8_t(datapool::TEST_UINT8,
			&testDataSet, PoolVariableIF::ReadWriteMode_t::VAR_READ_WRITE);
	gp_uint16_t testUint16 = gp_uint16_t(datapool::TEST_UINT16,
			&testDataSet, PoolVariableIF::ReadWriteMode_t::VAR_READ_WRITE);
	gp_uint32_t testUint32 = gp_uint32_t(datapool::TEST_UINT32,
			&testDataSet, PoolVariableIF::ReadWriteMode_t::VAR_READ_WRITE);
	gp_vec_t<float,2> testFloatVector = gp_vec_t<float, 2>(
			datapool::TEST_FLOAT_VECTOR, &testDataSet,
			PoolVariableIF::ReadWriteMode_t::VAR_READ_WRITE);

	SECTION("Reading") {
		REQUIRE(testDataSet.read() == retval::CATCH_OK);
		CHECK(testUint8.value == 0);
		CHECK(not testUint8.isValid());
		CHECK(testUint16.value == 0);
		CHECK(not testUint8.isValid());
		CHECK(testUint32.value == 0);
		CHECK(not testUint8.isValid());
		CHECK(testFloatVector.value[0] == 0.0);
		CHECK(testFloatVector.value[1] == 0.0);
		CHECK(not testUint8.isValid());
		CHECK(testDataSet.commit() == retval::CATCH_OK);
	}

	SECTION ("Writing") {
		REQUIRE(testDataSet.read() == retval::CATCH_OK);
		testUint8.value = 10;
		testUint16.value = 4284;
		testUint32.value = 2914921421;
		testFloatVector.value[0] = 0.82421;
		testFloatVector.value[1] = -221.4224;
		REQUIRE(testDataSet.commit() == retval::CATCH_OK);
	}

	SECTION ("Reading updated values") {
		REQUIRE(testDataSet.read() == retval::CATCH_OK);
		CHECK(testUint8.value == 10);
		CHECK(not testUint8.isValid());
		CHECK(testUint16.value == 4284);
		CHECK(not testUint8.isValid());
		CHECK(testUint32.value == 2914921421);
		CHECK(not testUint8.isValid());
		CHECK(testFloatVector.value[0] == Approx(0.82421));
		CHECK(testFloatVector.value[1] == Approx(-221.4224));
		CHECK(not testUint8.isValid());
		REQUIRE(testDataSet.commit() == retval::CATCH_OK);
	}

	SECTION("Writing different values and setting valid") {
		REQUIRE(testDataSet.read() == retval::CATCH_OK);
		testUint8.value = 116;
		testUint16.value = 21532;
		testUint32.value = 5;
		testFloatVector.value[0] = -0.002;
		testFloatVector.value[1] = 0.2321;
		REQUIRE(testDataSet.commit(true) == retval::CATCH_OK);
	}

	SECTION("Reading again") {
		REQUIRE(testDataSet.read() == retval::CATCH_OK);
		CHECK(testUint8.value == 116);
		CHECK(testUint8.isValid());

		CHECK(testUint16.value == 21532);
		CHECK(testUint16.isValid());

		CHECK(testUint32.value == 5);
		CHECK(testUint32.isValid());

		CHECK(testFloatVector.value[0] == Approx(-0.002));
		CHECK(testFloatVector.value[1] == Approx(0.2321));
		CHECK(testFloatVector.isValid());
		REQUIRE(testDataSet.commit(true) == retval::CATCH_OK);
	}
}
