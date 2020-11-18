/**
 * @file gpsPool.cpp
 *
 * @date 09.11.2019
 */

#include <stdint.h>
#include <stm32/fsfwconfig/hk/sid.h>

#ifndef CONFIG_DATAPOOL_GPSPOOL_H_
#define CONFIG_DATAPOOL_GPSPOOL_H_

namespace GpsInit{
struct navDataIdStruct { //!< [EXPORT] : [DATA]
	sid32_t sid;
	struct ParameterList {
		uint32_t fixModeId;
		uint32_t numberOfSvInFixId;
		uint32_t gnssWeekId;
		uint32_t timeOfWeekId;
		uint32_t latitudeId;
		uint32_t longitudeId;
		uint32_t meanSeaAltitudeId;
		uint32_t positionId;
		uint32_t velocityId;
	} __attribute__((packed));
	ParameterList p;
	uint8_t numberOfParameters = sizeof(p) / 4;
	float collectionInterval = 5;
	bool isDiagnosticsPacket = false;
	bool enablePacket = false;
};


/**
 * Initiate GPS pool structs with datapool IDs for HK service and GPSHandler
 * @param gps0
 * @param gps1
 */
void gpsIdStructInit(navDataIdStruct* gps0,navDataIdStruct* gps1);
}

#endif /* CONFIG_DATAPOOL_GPSPOOL_CPP_ */
