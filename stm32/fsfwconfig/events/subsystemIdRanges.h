/*
 * subsystemIdRanges.h
 *
 *  Created on: 16.07.2018
 *      Author: mohr
 */

#ifndef CONFIG_TMTC_SUBSYSTEMIDRANGES_H_
#define CONFIG_TMTC_SUBSYSTEMIDRANGES_H_

#include <cstdint>

/**
 * These IDs are part of the ID for an event thrown by a subsystem.
 * Numbers 0-80 are reserved for FSFW Subsystem IDs (framework/events/)
 */
namespace SUBSYSTEM_ID {
enum: uint8_t {
	/**
	 * 80-105: PUS Services
	 */
	PUS_SERVICE_2 = 82,
	PUS_SERVICE_3 = 83,
	PUS_SERVICE_5 = 85,
	PUS_SERVICE_6 = 86,
	PUS_SERVICE_8 = 88,
	PUS_SERVICE_23 = 91,
	DUMMY_DEVICE = 90,
	/**
	 * 105-115: AOCS
	 */
	GPS_DEVICE = 105,
    MGM_LIS3MDL = 130,

	SPI_COM_IF = 140,
	I2C_COM_IF = 141
};
}



#endif /* CONFIG_TMTC_SUBSYSTEMIDRANGES_H_ */
