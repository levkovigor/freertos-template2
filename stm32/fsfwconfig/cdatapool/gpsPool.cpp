/**
 * @file gpsPool.cpp
 *
 * @date 09.11.2019
 */
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfwconfig/cdatapool/dataPoolInit.h>
#include <stm32/fsfwconfig/cdatapool/gpsPool.h>
#include <stm32/fsfwconfig/hk/sid.h>

void GpsInit::gpsIdStructInit(navDataIdStruct* gps0,navDataIdStruct* gps1) {
	gps0->p.fixModeId = datapool::GPS0_FIX_MODE;
	gps0->p.gnssWeekId = datapool::GPS0_GNSS_WEEK;
	gps0->p.timeOfWeekId = datapool::GPS0_TIME_OF_WEEK;
	gps0->p.latitudeId = datapool::GPS0_LATITUDE;
	gps0->p.longitudeId = datapool::GPS0_LONGITUDE;
	gps0->p.meanSeaAltitudeId = datapool::GPS0_MEAN_SEA_ALTITUDE;
	gps0->p.numberOfSvInFixId = datapool::GPS0_NUMBER_OF_SV_IN_FIX;
	gps0->p.positionId = datapool::GPS0_POSITION;
	gps0->p.velocityId = datapool::GPS0_VELOCITY;

	const float GPS0_COLLECTION_INTERVAL = 5.0;
	gps0->sid = sid::GPS0_NAV_DATA;
	gps0->collectionInterval = GPS0_COLLECTION_INTERVAL;
	gps0->isDiagnosticsPacket = false;
	gps0->enablePacket = false;

	gps1->p.fixModeId = datapool::GPS1_FIX_MODE;
	gps1->p.gnssWeekId = datapool::GPS1_GNSS_WEEK;
	gps1->p.timeOfWeekId= datapool::GPS1_TIME_OF_WEEK;
	gps1->p.latitudeId = datapool::GPS1_LATITUDE;
	gps1->p.longitudeId = datapool::GPS1_LONGITUDE;
	gps1->p.meanSeaAltitudeId = datapool::GPS1_MEAN_SEA_ALTITUDE;
	gps1->p.numberOfSvInFixId = datapool::GPS1_NUMBER_OF_SV_IN_FIX;
	gps1->p.positionId = datapool::GPS1_POSITION;
	gps1->p.velocityId = datapool::GPS1_VELOCITY;

	const float GPS1_COLLECTION_INTERVAL = 0.4;
	gps1->sid = sid::GPS1_NAV_DATA;
	gps1->collectionInterval = GPS1_COLLECTION_INTERVAL;
	gps1->isDiagnosticsPacket = false;
	gps1->enablePacket = false;
}


