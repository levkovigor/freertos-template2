/*
 * AttitudeControllerAlgorithms.cpp
 *
 *  Created on: Feb 1, 2021
 *      Author: Mikael Senger
 */

#include <mission/controller/acs/AttitudeController.h>

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

void AttitudeController::calcQuatAndRefRot(double multFactor[QUATERNION_SIZE], double timeJulianDateUT1,
		double positionF[COORDINATES_SIZE], double velocityF[COORDINATES_SIZE],
		double quatRotPointingAxis[QUATERNION_SIZE],
		double positionTargetF[COORDINATES_SIZE],
		double quatCurrentAttitude[QUATERNION_SIZE],
		double quatLastPointing[QUATERNION_SIZE], timeval timeLastPointing) {

	/* Determining ECEF to ECI transformation matrix */
	double positionI[COORDINATES_SIZE], velocityI[COORDINATES_SIZE], positionTargetI[COORDINATES_SIZE];
	CoordinateTransformations::positionEcfToEci(positionF, positionI, &timeLastPointing);
	CoordinateTransformations::velocityEcfToEci(velocityF, positionF, velocityI, &timeLastPointing);
	CoordinateTransformations::positionEcfToEci(positionTargetF, positionTargetI, &timeLastPointing);

	double x[COORDINATES_SIZE];
	VectorOperations<double>::subtract(positionTargetI, positionI, x, 3);
	VectorOperations<double>::mulScalar(x, -1, x, 3);
	VectorOperations<double>::normalize(x, x, 3);

	double normalOrbit[COORDINATES_SIZE];
	VectorOperations<double>::cross(positionI, velocityI, normalOrbit);
	VectorOperations<double>::normalize(normalOrbit, normalOrbit, 3);

	double z[COORDINATES_SIZE];
	VectorOperations<double>::cross(x, normalOrbit, z);
	VectorOperations<double>::normalize(z, z, 3);

	double y[COORDINATES_SIZE];
	VectorOperations<double>::cross(z, x, y);
	VectorOperations<double>::normalize(y, y, 3);

	double matrixIX[3][3];
	matrixIX[0][0] = x[0];
	matrixIX[1][0] = x[1];
	matrixIX[2][0] = x[2];
	matrixIX[0][1] = y[0];
	matrixIX[1][1] = y[1];
	matrixIX[2][1] = y[2];
	matrixIX[0][2] = z[0];
	matrixIX[1][2] = z[1];
	matrixIX[2][2] = z[2];

	uint8_t indexMaxElement = 0;

	QuaternionOperations::fromDcm(matrixIX, quatLastPointing, &indexMaxElement);
	QuaternionOperations::normalize(quatLastPointing);

	VectorOperations<double>::mulScalar(quatLastPointing, multFactor[indexMaxElement], quatLastPointing, 4);

	for(uint8_t i = 0; i < 4; i++) {
		if(quatLastPointing[i] < 0) {
			multFactor[i] = -1;
		} else {
			multFactor[i] = 1;
		}
	}

	QuaternionOperations::multiply(quatLastPointing, quatRotPointingAxis, quatLastPointing);

	double quatBX[QUATERNION_SIZE];

	QuaternionOperations::multiply(quatCurrentAttitude, quatLastPointing, quatBX);
	QuaternionOperations::normalize(quatBX);


/*

	double quaternionInertialToTarget[4]; qXI

	QuaternionOperations::inverse(quaternionTargetToInertial,
			quaternionInertialToTarget);

	rotationFromQuaternions.update(quaternionInertialToTarget, acsTime,
			inertialRotationOfTarget);

	for (uint8_t i = 0; i < 3; i++) {
		inertialRotationOfTarget[i] = inertialRotationOfTarget[i] * alpha
				+ lastInertialRotationOfTarget[i] * (1 - alpha);
		lastInertialRotationOfTarget[i] = inertialRotationOfTarget[i];
	}
*/
}
