/*
 * RotationCalc.cpp
 *
 *  Created on: Feb 19, 2021
 *      Author: user
 */

#include <fsfw/globalfunctions/math/QuaternionOperations.h>
#include <fsfw/globalfunctions/math/VectorOperations.h>
#include "RotationCalc.h"

RotationCalc::RotationCalc() :
		timeOfLastQuaternion( { 0, 0 }), valid(false) {
	lastQuaternion = {0,0,0,1};
}

RotationCalc::~RotationCalc() {
}

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
			//No Rotation
			rotation[0] = 0;
			rotation[1] = 0;
			rotation[2] = 0;
		}else{
			//Get Rotation axis
			VectorOperations<double>::normalize(quatLastXI, rotation, 3);
			//Get linear rotation over time
			VectorOperations<double>::mulScalar(rotation, angle / deltaT,
					rotation, 3);
		}
	} else {
		valid = true;
		rotation[0] = 0;
		rotation[1] = 0;
		rotation[2] = 0;
	}

	memcpy(lastQuaternion, quaternion, 4 * sizeof(*quaternion));
	lastQuaternion[0] = quaternion[0];
	lastQuaternion[1] = quaternion[1];
	lastQuaternion[2] = quaternion[2];
	lastQuaternion[3] = quaternion[3];
	timeOfLastQuaternion = timeOfQuaternion;

}
