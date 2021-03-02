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

std::array<uint8_t, sizeof(CriticalDataBlock)> FRAMHandler::criticalBlock;

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

void FRAMHandler::printCriticalBlock() {
    /* Read the critical block */
    int result = read_critical_block(criticalBlock.data(), sizeof(CriticalDataBlock));
    if(result != 0) {
        return;
    }

    size_t critBlockSize = sizeof(CriticalDataBlock);
    uint16_t critBlockPages = critBlockSize / 4;
    uint16_t lastPageBytes = critBlockSize % 4;
    uint16_t currentPage = 0;

    sif::printInfo("Printing critical block:\n\r");
    for(currentPage = 0; currentPage < critBlockPages; currentPage++) {
        sif::printInfo("Page %hu: 0x%02x, 0x%02x, 0x%02x, 0x%02x\n\r",
                currentPage + 1, criticalBlock[currentPage * 4],
                criticalBlock[currentPage * 4 + 1],
                criticalBlock[currentPage * 4 + 2],
                criticalBlock[currentPage * 4 + 3]);
    }

    if(lastPageBytes > 0) {
        char lastPage[30];
        sprintf(lastPage, "Page %hu: ", currentPage + 1);
        for(uint8_t idx = 0; idx < lastPageBytes; idx++) {
            if(idx == lastPageBytes - 1) {
                sprintf(lastPage, "0x%0x2x\n\r", criticalBlock[currentPage * 4 + idx]);
            }
            else {
                sprintf(lastPage, "0x%0x2x, ", criticalBlock[currentPage * 4 + idx]);
            }
        }
        sif::printInfo("%s\n\r", lastPage);
    }
}

ReturnValue_t FRAMHandler::initialize() {
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
    /* Write software version and subversion to FRAM */
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

void FRAMHandler::dumpCriticalBlock() {
}
