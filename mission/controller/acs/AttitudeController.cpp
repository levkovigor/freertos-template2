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

/*AttitudeController::AttitudeController(object_id_t objectId) :
        ExtendedControllerBase(objectId, objects::NO_OBJECT), acsParameters(
                ACS_PARAMETERS_PARAMETER_DOMAIN_ID) {
}*/

AttitudeController::~AttitudeController() {
}

ReturnValue_t AttitudeController::handleCommandMessage(CommandMessage *message) {
    return HasReturnvaluesIF::RETURN_OK;
}

// Periodic function that will later control the attitude
void AttitudeController::performControlOperation() {
    timeval now = { 0, 0 };
//    OSAL::getClock_timeval(&now);

    timeval acsTime = { 0, 0 };
    acsTime += now;
    double quatBX[4] = { 0, 0, 0, 1 };
    double rotationRate[3] = { 0, 0, 0 };
    double positionTargetI[3] = { 0,0,0};
    double quatLastPointing[4] = { 0, 0, 0, 1 };
    static_cast<void>(quatBX);
    static_cast<void>(rotationRate);
    static_cast<void>(positionTargetI);
    static_cast<void>(quatLastPointing);
#if OBSW_ACS_TEST == 1
    // test code work in progress
    calcQuatAndRefRot(timevalOperations::toTimeval(currentTimeJDTest[1]), positionFTest[1],
            velocityFTest[1], quatRotPointingAxisTest[1], positionTargetFTest[1],
            quatCurrentAttitudeTest, positionTargetI, quatLastPointing, quatBX, rotationRate);

    // Quaternion Last Pointing Attitude
    sif::printInfo("%20s: | %10.4f | %10.4f | %10.4f | %10.4f |\n", "Test Output",
            quatLastPointingTestList[3][0], quatLastPointingTestList[3][1],
            quatLastPointingTestList[3][2], quatLastPointingTestList[3][3]);
    sif::printInfo("%20s: | %10.4f | %10.4f | %10.4f | %10.4f |\n\n", "Code Output",
            quatLastPointing[0], quatLastPointing[1], quatLastPointing[2], quatLastPointing[3]);

    // Reference Rotation Rate
    sif::printInfo("%20s: | %10.4f | %10.4f | %10.4f | %10s |\n", "Test Output",
            rotationRateReferenceTest[1][0], rotationRateReferenceTest[1][1],
            rotationRateReferenceTest[1][2], "N/A");
    sif::printInfo("%20s: | %10.4f | %10.4f | %10.4f | %10s |\n\n", "Code Output",
            rotationRate[0], rotationRate[1], rotationRate[2], "N/A");

    /*#if FSFW_CPP_OSTREAM_ENABLED == 0
     sif::printInfo("Hallo Welt\n");
     #else
     sif::info<<"%f; %f; %f\n", refRotRateTest[0], refRotRateTest[1],
     refRotRateTest[2]<<std::endl;
     #endif*/
#endif
}

ReturnValue_t AttitudeController::checkModeCommand(Mode_t mode, Submode_t submode,
        uint32_t *msToReachTheMode) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t AttitudeController::initializeAfterTaskCreation() {
    return HasReturnvaluesIF::RETURN_OK;
}

void AttitudeController::handleChangedDataset(sid_t sid, store_address_t storeId,
        bool *clearMessage) {
}

ReturnValue_t AttitudeController::initializeLocalDataPool(localpool::DataPool &localDataPoolMap,
        LocalDataPoolManager &poolManager) {
    return HasReturnvaluesIF::RETURN_OK;
}

LocalPoolDataSetBase* AttitudeController::getDataSetHandle(sid_t sid) {
    return nullptr;
}
