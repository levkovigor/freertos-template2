/**
 * @author Mikael Senger
 *
 * About: Source file of the AttitudeController class, whose objective it is to
 * manage the attitude of SOURCE and calculate the rotation needed to target a
 * point
 */

#include "AttitudeController.h"

#include <fsfw/serviceinterface/ServiceInterface.h>
#include <fsfw/coordinates/CoordinateTransformations.h>
#include <fsfw/globalfunctions/constants.h>
#include <fsfw/globalfunctions/math/MatrixOperations.h>
#include <fsfw/globalfunctions/math/QuaternionOperations.h>
#include <fsfw/globalfunctions/math/VectorOperations.h>
#include <fsfw/globalfunctions/sign.h>
#include <fsfw/subsystem/SubsystemBase.h>
#include <string.h>

#include <fsfw/coordinates/CoordinateTransformations.h>

AttitudeController::AttitudeController(object_id_t objectId) :
		ExtendedControllerBase(objectId, objects::NO_OBJECT) {
}

AttitudeController::~AttitudeController() {
}

ReturnValue_t AttitudeController::handleCommandMessage(
		CommandMessage *message) {
	return HasReturnvaluesIF::RETURN_OK;
}

// Periodic function that will later control the attitude
void AttitudeController::performControlOperation() {
#if OBSW_ACS_TEST == 1
	// test code work in progress
	//sif::printInfo("%f; %f; %f; %f\n", multFactorTest[0][0],
	//		multFactorTest[0][1], multFactorTest[0][2], multFactorTest[0][3]);
	calcQuatAndRefRot(multFactorTest[2],
			timevalOperations::toTimeval(currentTimeJDTest[2]),
			positionFTest[2], velocityFTest[2], quatRotPointingAxisTest[2],
			positionTargetFTest[2], quatCurrentAttitudeTest[2],
			positionTargetITest, quatLastPointingTest[2], quatBXTest,
			refRotRateTest);
#if FSFW_CPP_OSTREAM_ENABLED == 1
	sif::info<<"Hallo Welt"<<std::endl;
#else
	sif::printInfo("%f; %f; %f\n", refRotRateTest[0], refRotRateTest[1],
			refRotRateTest[2]);
#endif
#endif
}

ReturnValue_t AttitudeController::checkModeCommand(Mode_t mode,
		Submode_t submode, uint32_t *msToReachTheMode) {
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t AttitudeController::initializeAfterTaskCreation() {
	return HasReturnvaluesIF::RETURN_OK;
}

void AttitudeController::handleChangedDataset(sid_t sid,
		store_address_t storeId, bool *clearMessage) {
}

ReturnValue_t AttitudeController::initializeLocalDataPool(
		localpool::DataPool &localDataPoolMap,
		LocalDataPoolManager &poolManager) {
	return HasReturnvaluesIF::RETURN_OK;
}

LocalPoolDataSetBase* AttitudeController::getDataSetHandle(sid_t sid) {
	return nullptr;
}
