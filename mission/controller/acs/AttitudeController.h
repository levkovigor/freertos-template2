/*
 * AttitudeController.h
 *
 *  Created on: Feb 1, 2021
 *      Author: Mikael Senger
 */

#ifndef MISSION_CONTROLLER_ATTITUDECONTROLLER_H_
#define MISSION_CONTROLLER_ATTITUDECONTROLLER_H_


#include <fsfw/action/HasActionsIF.h>
#include <fsfw/action/SimpleActionHelper.h>
#include <fsfw/controller/ExtendedControllerBase.h>
#include <fsfw/health/HealthTableIF.h>
#include <fsfw/parameters/ParameterHelper.h>
#include <fsfw/parameters/ReceivesParameterMessagesIF.h>
#include <mission/controller/acs/helpfunctions/RotationCalc.h>


class AttitudeController: public ExtendedControllerBase {

public:

	AttitudeController(object_id_t objectId);

	virtual ~AttitudeController();

	static constexpr uint8_t COORDINATES_SIZE = 3;

	static constexpr uint8_t QUATERNION_SIZE = 4;


protected:

    void calcQuatAndRefRot(double *multFactor,
    			timeval currentTime,const double *positionF,
    			const double *velocityF,
				double *quatRotPointingAxis,
    			const double *positionTargetF,
    			double *quatCurrentAttitude,
				double *positionTargetI,
    			double *quatLastPointing,
    			double *quatBX,
    			double *refRotRate);

    int8_t multFactor[QUATERNION_SIZE] {1,1,1,1};

	RotationCalc rotationCalc;

private:

	// TODO: Add stubs for thermal components. Each device / assembly with one
	// or multiple redundant sensors will have a thermal component.

	/** ExtendedControllerBase overrides */
    virtual ReturnValue_t handleCommandMessage(
            CommandMessage *message) override;

    virtual void performControlOperation() override;

    virtual ReturnValue_t checkModeCommand(Mode_t mode, Submode_t submode,
            uint32_t *msToReachTheMode) override;

    ReturnValue_t initializeAfterTaskCreation() override;

    void handleChangedDataset(sid_t sid, store_address_t storeId) override;

    double lastRefRotRate[COORDINATES_SIZE] {0,0,0};
};

/*
typedef struct input{
	double multFactor[QUATERNION_SIZE];
	/ sign correction /
	double timeJulianDateUT1;
	double positionF[COORDINATES_SIZE];
	/ F: ECEF (earth-centered, earth-fixed) coordinate system /
	double velocityF[COORDINATES_SIZE];
	double quatRotPointingAxis[QUATERNION_SIZE];
	/ quatRef /
	double positionTargetF[COORDINATES_SIZE];
	double quatCurrentAttitude[QUATERNION_SIZE];
	/ quatBI /
	double quatLastPointing[QUATERNION_SIZE];
	/ quatIX /
	double timeLastPointing;
}Input;

typedef struct output{
	double quatBX[QUATERNION_SIZE];
	/ B: Body coordinate System X:Target coordinate System /
	double rotRateRef[COORDINATES_SIZE];
	double quatCurrentPointing[QUATERNION_SIZE];
	/ quatIX /
	double timeCurrentPointing;
	double multFactor[QUATERNION_SIZE];
	double quatIX[QUATERNION_SIZE];
	/ I: ECI (earth-centered inertial) coordinate system /
	double positionTargetI[COORDINATES_SIZE];
	double vectorOppToTargetI[COORDINATES_SIZE];
}Output;
*/

#endif /* MISSION_CONTROLLER_ATTITUDECONTROLLER_H_ */
