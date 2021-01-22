#include "FRAMHandler.h"
#include <fsfw/serviceinterface/ServiceInterface.h>
#include <fsfw/timemanager/Stopwatch.h>

#ifdef ISIS_OBC_G20
#include <hal/Storage/FRAM.h>
#else
#include <sam9g20/common/VirtualFRAMApi.h>
#include <sam9g20/memory/SDCardAccess.h>
#endif


#include <fsfwconfig/OBSWVersion.h>

FRAMHandler::FRAMHandler(object_id_t objectId): SystemObject(objectId) {
}

FRAMHandler::~FRAMHandler() {}

ReturnValue_t FRAMHandler::handleMemoryLoad(uint32_t address,
		const uint8_t *data, uint32_t size, uint8_t **dataPointer) {
	return RETURN_OK;
}

ReturnValue_t FRAMHandler::handleMemoryDump(uint32_t address, uint32_t size,
		uint8_t** dataPointer, uint8_t* dumpTarget) {
	return RETURN_OK;
}

ReturnValue_t FRAMHandler::setAddress(uint32_t *startAddress) {
	return RETURN_OK;
}

ReturnValue_t FRAMHandler::initialize() {
    Stopwatch watch;
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "FRAMHandler: Starting FRAM component." << std::endl;
#else
    sif::printInfo("FRAMHandler: Starting FRAM component.\n");
#endif

#ifdef AT91SAM9G20_EK
    SDCardAccess access;
#endif

    int result = FRAM_start();
    if(result != 0) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    // Write software version and subversion to FRAM
    result = write_software_version(SW_VERSION, SW_SUBVERSION,
            SW_SUBSUBVERSION);
    if(result != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "FRAMHandler::initialize: Error writing software version "
                "to FRAM" << std::endl;
#else
        sif::printError("FRAMHandler::initialize: Error writing software version "
                "to FRAM\n");
#endif
    }
    // sif::printInfo("FRAM maximum address: %d\n\r", FRAM_getMaxAddress());
    return HasReturnvaluesIF::RETURN_OK;
}
