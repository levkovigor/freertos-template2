/**
 * @file hkInit.h
 *
 * @date 26.12.2019
 * @author R. Mueller
 */

#ifndef CONFIG_HK_HKINIT_H_
#define CONFIG_HK_HKINIT_H_

#include <config/cdatapool/gpsPool.h>
#include <config/cdatapool/testPool.h>
#include <fsfw/pus/Service3Housekeeping.h>

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
void hkInit(Service3Housekeeping * housekeepingService, struct hk::hkIdStruct hkIdStruct);

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
