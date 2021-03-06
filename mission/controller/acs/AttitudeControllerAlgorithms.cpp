/**
 * @author Mikael Senger
 *
 * About: Algorithms needed for the AttitudeController class
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

/* Guidance algorithm function, uses global functions math operations and
 RotationCalc class*/
void AttitudeController::calcQuatAndRefRot(timeval currentTime, const double *positionF,
        const double *velocityF, double *quatRotPointingAxis, const double *positionTargetF,
        double *quatCurrentAttitude, double *positionTargetI, double *quatLastPointing,
        double *quatBX, double *refRotRate) {

    // Determining ECEF (short F) to ECI (short I) transformation matrix
    double positionI[COORDINATES_SIZE], velocityI[COORDINATES_SIZE];
    CoordinateTransformations::positionEcfToEci(positionF, positionI, &currentTime);
    CoordinateTransformations::velocityEcfToEci(velocityF, positionF, velocityI, &currentTime);
    CoordinateTransformations::positionEcfToEci(positionTargetF, positionTargetI, &currentTime);

    // Creating target coordinate system (short X)
    double x[COORDINATES_SIZE];
    VectorOperations<double>::subtract(positionTargetI, positionI, x, 3);
    /* SOURCE points towards the target with the opposite direction of the
     x-Axis, due to camera position */
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

    // I to X coordinate matrix
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

    VectorOperations<double>::mulScalar(quatLastPointing, multFactor[indexMaxElement],
            quatLastPointing, 4);

    /* correct the sign of the quaternion components to ensure the correct sign
     of the reference rotation rate */
    for (uint8_t i = 0; i < 4; i++) {
        if (quatLastPointing[i] < 0) {
            multFactor[i] = -1;
        } else {
            multFactor[i] = 1;
        }
    }

    // Rotate around pointing axis
    QuaternionOperations::multiply(quatLastPointing, quatRotPointingAxis, quatLastPointing);

    QuaternionOperations::inverse(quatCurrentAttitude, quatCurrentAttitude);
    QuaternionOperations::multiply(quatCurrentAttitude, quatLastPointing, quatBX);
    QuaternionOperations::normalize(quatBX);

    rotationCalc.calc(quatLastPointing, currentTime, refRotRate);

    /* Pending calculation method of reference rate
     for (uint8_t i = 0; i < 3; i++) {
     refRotRate[i] = refRotRate[i] * alpha
     + lastRefRotRate[i] * (1 - alpha);
     lastRefRotRate[i] = refRotRate[i];
     }
     */

}
