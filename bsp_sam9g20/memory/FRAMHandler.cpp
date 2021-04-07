#include "FRAMHandler.h"
#include <fsfw/serviceinterface/ServiceInterface.h>
#include <fsfw/timemanager/Stopwatch.h>

#ifdef ISIS_OBC_G20
#include <hal/Storage/FRAM.h>
#else
#include <bsp_sam9g20/common/fram/VirtualFRAMApi.h>
#include <bsp_sam9g20/memory/SDCardAccess.h>
#endif


#include <OBSWVersion.h>

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

ReturnValue_t FRAMHandler::readBootloaderBlock(uint8_t** ptr) {
    int result = fram_read_bootloader_block_raw(criticalBlock.data(), sizeof(BootloaderGroup));
    if(result != 0) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    if(ptr != nullptr) {
        *ptr = criticalBlock.data();
    }
    return HasReturnvaluesIF::RETURN_FAILED;
}

void FRAMHandler::printBootloaderBlock() {
    int result = fram_read_bootloader_block_raw(criticalBlock.data(), sizeof(BootloaderGroup));
    if(result != 0) {
        return;
    }

    size_t blBlockSize = sizeof(BootloaderGroup);
    sif::printInfo("Printing bootloader block:\n");
    genericBlockPrinter(blBlockSize);
}

void FRAMHandler::printCriticalBlock() {
    /* Read the critical block */
    int result = fram_read_critical_block(criticalBlock.data(), sizeof(CriticalDataBlock));
    if(result != 0) {
        return;
    }

    size_t critBlockSize = sizeof(CriticalDataBlock);
    sif::printInfo("Printing critical block:\n");
    genericBlockPrinter(critBlockSize);
}

void FRAMHandler::genericBlockPrinter(size_t blockSize) {
    uint16_t blockPages = blockSize / 4;
    uint16_t lastPageBytes = blockSize % 4;
    uint16_t currentPage = 0;
    for(currentPage = 0; currentPage < blockPages; currentPage++) {
        sif::printInfo("Page %hu: 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
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
    result = fram_write_software_version(SW_VERSION, SW_SUBVERSION,
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

ReturnValue_t FRAMHandler::zeroOutDefaultZeroFields(int* errorField) {
    int result = fram_zero_out_default_zero_fields();
    if(result != 0) {
        if(errorField != nullptr) {
            *errorField = result;
        }
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    return HasReturnvaluesIF::RETURN_OK;
}
