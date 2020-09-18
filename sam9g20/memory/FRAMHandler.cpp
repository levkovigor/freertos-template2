#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <sam9g20/memory/FRAMHandler.h>

extern "C" {
#include <hal/Storage/FRAM.h>
}

#include <version.h>

FRAMHandler::FRAMHandler(object_id_t objectId): SystemObject(objectId) {
}

FRAMHandler::~FRAMHandler() {
}

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
    sif::info << "FRAMHandler: Starting FRAM component." << std::endl;
    int result = FRAM_start();
    if(result != 0) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    // Write software version and subversion to FRAM
    result = write_software_version(SW_VERSION, SW_SUBVERSION);
    if(result != 0) {
        sif::error << "FRAMHandler::initialize: Error writing software version "
                "to FRAM" << std::endl;
    }
    return HasReturnvaluesIF::RETURN_OK;
}
