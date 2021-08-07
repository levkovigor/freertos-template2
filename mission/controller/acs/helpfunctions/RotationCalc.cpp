/**
 * @author Mikael Senger
 *
 * About: Class used to store information about last instance of the guidance
 * algorithm and to calculate reference rotation
 */

#include "RotationCalc.h"

#include <fsfw/globalfunctions/math/QuaternionOperations.h>
#include <fsfw/globalfunctions/math/VectorOperations.h>

RotationCalc::RotationCalc() :
		timeOfLastQuaternion( { 0, 0 }), valid(false) {
}

RotationCalc::~RotationCalc() {
}

// Function calculates reference rotation rate to target
void RotationCalc::calc(const double* quaternion,
		const timeval timeOfQuaternion, double* rotation) {

	if (valid) {
		double quatCurrentXI[4];
		double quatLastXI[4];

		double deltaT = timevalOperations::toDouble(
				timeOfQuaternion - timeOfLastQuaternion);

		QuaternionOperations::inverse(quaternion, quatCurrentXI);

		QuaternionOperations::inverse(lastQuaternion, quatLastXI);

		QuaternionOperations::multiply(quatCurrentXI, quatLastXI, quatLastXI);

		QuaternionOperations::normalize(quatLastXI);

		double angle = QuaternionOperations::getAngle(quatLastXI);

		if(angle==0){
			//No rotation relative to target
			rotation[0] = 0;
			rotation[1] = 0;
			rotation[2] = 0;
		}else{
			//Get rotation axis
			VectorOperations<double>::normalize(quatLastXI, rotation, 3);

			//Get linear rotation rate over time
			VectorOperations<double>::mulScalar(rotation, angle / deltaT,
					rotation, 3);
		}
	} else {
		// For the first instance of function
		valid = true;
		rotation[0] = 0;
		rotation[1] = 0;
		rotation[2] = 0;
	}

	// Saving of new quaternion and time for next instance
	memcpy(lastQuaternion, quaternion, 4 * sizeof(*quaternion));
	timeOfLastQuaternion = timeOfQuaternion;
}

void RotationCalc::reset() {
	lastQuaternion[0] = 0;
	lastQuaternion[1] = 0;
	lastQuaternion[2] = 0;
	lastQuaternion[3] = 1;
	timeOfLastQuaternion = {0,0};
	valid = false;
}
