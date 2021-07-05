/**
 * @author Mikael Senger
 *
 * About: Source file of the AttitudeController class, whose objective it is to
 * manage the attitude of SOURCE and calculate the rotation needed to target a
 * point
 */

#include "AttitudeController.h"

#include <fsfw/serviceinterface/ServiceInterface.h>
#include <fsfw/timemanager/Clock.h>
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
        ExtendedControllerBase(objectId, objects::NO_OBJECT), acsParameters(
                ACS_PARAMETERS_PARAMETER_DOMAIN_ID) {
}

AttitudeController::~AttitudeController() {
}

ReturnValue_t AttitudeController::handleCommandMessage(CommandMessage *message) {
    return HasReturnvaluesIF::RETURN_OK;
}

// Periodic function that will later control the attitude
void AttitudeController::performControlOperation() {
    timeval now = { 0, 0 };
    Clock::getClock_timeval(&now);
    //timeval acsTime = { 0, acsParameters.acsTimeOffset_ms * 1000 };
    //acsTime += now;
    double quatBX[4] = { 0, 0, 0, 1 };
    double rotationRate[3] = { 0, 0, 0 };
    double positionTargetI[3] = { 0,0,0};
    double quatLastPointing[4] = { 0, 0, 0, 1 };
#if OBSW_ACS_TEST == 1
    // test code work in progress
    if(currentTestTime <= 1e-10)
    currentTestTime = currentTimeJDTest[1];

    calcQuatAndRefRot(timevalOperations::toTimeval(currentTestTime), positionFTest[1],
            velocityFTest[1], quatRotPointingAxisTest[1], positionTargetFTest[1],
            quatCurrentAttitudeTest, positionTargetI, quatLastPointing, quatBX, rotationRate);

    currentTestTime = currentTestTime + testTimeInterval;

    // Quaternion Last Pointing Attitude
    sif::printInfo("%25s: | %10.4f | %10.4f | %10.4f | %10.4f |\n", "Test Last Pointing Quaternion",
            0.996192286201972, -0.00253193611993782, -0.000789664059530919, 0.0871429552049189);
    sif::printInfo("%25s: | %10.4f | %10.4f | %10.4f | %10.4f |\n\n", "Code Last Pointing Quaternion",
            quatLastPointing[0], quatLastPointing[1], quatLastPointing[2], quatLastPointing[3]);

    // Reference Rotation Rate
    sif::printInfo("%25s: | %10.4E | %10.4E | %10.4E | %10s |\n", "Test Rotation Rate",
            -3.82254762478599e-14, 9.71544001638247e-17, 3.03006609963529e-17, "N/A");
    sif::printInfo("%25s: | %10.4E | %10.4E | %10.4E | %10s |\n\n", "Code Rotation Rate",
            rotationRate[0], rotationRate[1], rotationRate[2], "N/A");

    // Quaternion Target to Body
    sif::printInfo("%25s: | %10.4E | %10.4E | %10.4E | %10.4E |\n", "Test quatBX",
            0.996192299293808, -0.00253178785805783, -0.000788980479093517, 0.0871428160421915);
    sif::printInfo("%25s: | %10.4E | %10.4E | %10.4E | %10.4E |\n\n", "Code quatBX", quatBX[0],
            quatBX[1], quatBX[2], quatBX[3]);

    // Quaternion Position Target ECI
    sif::printInfo("%25s: | %10.4E | %10.4E | %10.4E | %10s |\n", "Test Position Target ECI", 0.0, 0.0, 0.0, "N/A");
    sif::printInfo("%25s: | %10.4E | %10.4E | %10.4E | %10s |\n\n\n", "Code Position Target ECI", positionTargetI[0],
            positionTargetI[1], positionTargetI[2], "N/A");

    //sif::printInfo("%f\n", currentTestTime);

    /*#if FSFW_CPP_OSTREAM_ENABLED == 0
     sif::printInfo("Hallo Welt\n");
     #else
     sif::info<<"%f; %f; %f\n", refRotRateTest[0], refRotRateTest[1],
     refRotRateTest[2]<<std::endl;
     #endif*/
#endif
}

MessageQueueId_t AttitudeController::getCommandQueue() const {
    return commandQueue->getId();
}

ReturnValue_t AttitudeController::getParameter(uint8_t domainId, uint8_t uniqueIdentifier,
                ParameterWrapper *parameterWrapper, const ParameterWrapper *newValues,
                uint16_t startAtIndex) {
    return acsParameters.getParameter(domainId, uniqueIdentifier, parameterWrapper, newValues,
            startAtIndex);
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
