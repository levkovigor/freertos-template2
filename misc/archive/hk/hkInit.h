#ifndef CONFIG_HK_HKINIT_H_
#define CONFIG_HK_HKINIT_H_

#include "gpsPool.h"
#include "testPool.h"
#include <mission/pus/Service3HousekeepingPSB.h>

namespace hk {

struct hkIdStruct {
	GpsInit::navDataIdStruct gps0;
	GpsInit::navDataIdStruct gps1;
	TestInit::TestIdStruct test;
};

/**
 * Initialise all pre-defined housekeeping definitions
 * @param housekeepingService Housekeeping Service 3 instance
 * @param hkIdStruct Contains all HK definitions as structs
 */
void hkInit(Service3HousekeepingPSB * housekeepingService,
		struct hk::hkIdStruct hkIdStruct);

/**
 * Initiate HK ID struct with local ID structs
 * @param hkIdStruct
 * @param gps0
 * @param gps1
 */
void initHkStruct(struct hkIdStruct * hkIdStruct,
		GpsInit::navDataIdStruct gps0,GpsInit::navDataIdStruct gps1,
		TestInit::TestIdStruct test);

}

#endif /* CONFIG_HK_HKINIT_H_ */
