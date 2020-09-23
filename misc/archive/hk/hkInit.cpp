#include "dataPoolInit.h"
#include <config/hk/hkInit.h>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>
#include <mission/pus/servicepackets/Service3PsbPackets.h>

//// TODO: Generate this file automatically from header file
void hk::initHkStruct(struct hkIdStruct * hkIdStruct,
		GpsInit::navDataIdStruct gps0,GpsInit::navDataIdStruct gps1,
		TestInit::TestIdStruct test) {
	hkIdStruct->gps0 = gps0;
	hkIdStruct->gps1 = gps1;
	hkIdStruct->test = test;
}

void hk::hkInit(Service3HousekeepingPSB * housekeepingService,
		struct hk::hkIdStruct hkIdStruct) {
	HkPacketDefinition hkPacket;
	ReturnValue_t initResult;
	/* GPS 0 */
	hkPacket.initialize(
			hkIdStruct.gps0.sid,
			hkIdStruct.gps0.collectionInterval,
			hkIdStruct.gps0.numberOfParameters,
			reinterpret_cast<uint32_t *>(&hkIdStruct.gps0.p),
			hkIdStruct.gps0.enablePacket
	);
	initResult = housekeepingService->addPacketDefinitionToStore(&hkPacket,
			hkIdStruct.gps0.isDiagnosticsPacket);
	if(initResult != HasReturnvaluesIF::RETURN_OK) {
		sif::debug << "Inserting GPS0 HK definition failed with ID " <<
				initResult << std::endl;
	}

	/* GPS 1 */
	hkPacket.initialize(
			hkIdStruct.gps1.sid,
			hkIdStruct.gps1.collectionInterval,
			hkIdStruct.gps1.numberOfParameters,
			reinterpret_cast<uint32_t *>(&hkIdStruct.gps1.p),
			hkIdStruct.gps1.enablePacket
	);
	initResult = housekeepingService->addPacketDefinitionToStore(&hkPacket,
			hkIdStruct.gps1.isDiagnosticsPacket);
	if(initResult != HasReturnvaluesIF::RETURN_OK) {
		sif::debug << "Inserting GPS1 HK definition failed with ID " <<
				initResult << std::endl;
	}

	/* Test */
	hkPacket.initialize(
			hkIdStruct.test.sid,
			hkIdStruct.test.collectionInterval,
			hkIdStruct.test.numberOfParameters,
			reinterpret_cast<uint32_t *>(&hkIdStruct.test.p),
			hkIdStruct.test.enablePacket
	);
	initResult = housekeepingService->addPacketDefinitionToStore(&hkPacket,
			hkIdStruct.test.isDiagnosticsPacket);
	if(initResult != HasReturnvaluesIF::RETURN_OK) {
		sif::debug << "Inserting Test HK definition failed with ID " <<
				initResult << std::endl;
	}
}

