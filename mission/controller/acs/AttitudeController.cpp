/*
 * AttitudeController.cpp
 *
 *  Created on: Feb 1, 2021
 *      Author: Mikael Senger
 */

#include "AttitudeController.h"

#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/coordinates/CoordinateTransformations.h>
#include <fsfw/globalfunctions/constants.h>
#include <fsfw/globalfunctions/math/MatrixOperations.h>
#include <fsfw/globalfunctions/math/QuaternionOperations.h>
#include <fsfw/globalfunctions/math/VectorOperations.h>
#include <fsfw/globalfunctions/sign.h>
#include <fsfw/subsystem/SubsystemBase.h>
#include <string.h>
#include <fsfw/coordinates/CoordinateTransformations.h>
#include <OBSWConfig.h>

AttitudeController::AttitudeController(object_id_t objectId) :
		ExtendedControllerBase(objectId, objects::NO_OBJECT) {
}

AttitudeController::~AttitudeController() {
}

ReturnValue_t AttitudeController::handleCommandMessage(
		CommandMessage *message) {
	return HasReturnvaluesIF::RETURN_OK;
}

void AttitudeController::performControlOperation() {
	//sif::info;
#if OBSW_ACS_TEST == 1

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
		store_address_t storeId) {
}
