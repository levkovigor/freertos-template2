#include <mission/memory/FRAMHandler.h>


FRAMHandler::FRAMHandler() {
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





