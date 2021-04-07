/**
 * @author Mikael Senger
 *
 * About: Class used to store information about last instance of the guidance
 * algorithm and to calculate reference rotation
 */

#ifndef MISSION_CONTROLLER_ACS_HELPFUNCTIONS_ROTATIONCALC_H_
#define MISSION_CONTROLLER_ACS_HELPFUNCTIONS_ROTATIONCALC_H_


#include <fsfw/globalfunctions/timevalOperations.h>
#include <cmath>
#include <cstring>
#include <stdint.h>

class RotationCalc {
public:
	RotationCalc();
	virtual ~RotationCalc();

	void calc(const double *quaternion,const timeval timeOfQuaternion,
			double *rotation);
	void reset();

	double lastQuaternion[4] { 0, 0, 0, 1 };
	timeval timeOfLastQuaternion;
	bool valid;
};

#endif /* MISSION_CONTROLLER_ACS_HELPFUNCTIONS_ROTATIONCALC_H_ */
