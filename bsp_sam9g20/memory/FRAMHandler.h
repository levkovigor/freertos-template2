#ifndef SAM9G20_MEMORY_FRAMHANDLER_H_
#define SAM9G20_MEMORY_FRAMHANDLER_H_

#include "OBSWConfig.h"

#include "bsp_sam9g20/common/fram/FRAMApi.h"
#include "bsp_sam9g20/common/fram/CommonFRAM.h"

#include <fsfw/memory/HasMemoryIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>
#include <fsfw/serialize/SerialLinkedListAdapter.h>
#include <fsfw/tasks/ExecutableObjectIF.h>

#include <array>

/**
 * Additional abstraction layer to encapsulate access to FRAM
 */
class FRAMHandler : public SystemObject,
        public HasMemoryIF,
        public HasReturnvaluesIF {
public:
	FRAMHandler(object_id_t objectId);
	virtual~ FRAMHandler();

	/**
	 * Read the bootloader block into the private array and assign a given pointer to that
	 * array
	 * @param ptr Address of a pointer which will be assigned.
	 */
	static ReturnValue_t readBootloaderBlock(uint8_t** ptr);

	static void dumpCriticalBlock();

	/**
	 * Print the bootloader block, byte-wise, 4 bytes per line.
	 */
	static void printBootloaderBlock();
	/**
	 * Print the critical block, byte-wise, 4 bytes per line. For debugging purpose,
	 * do not use in mission code. It will later be possible to also dump the critical block
	 * as a binary file.
	 */
	static void printCriticalBlock();

	/**
	 * Can be called if the FRAM has been reset or is uninitialized. Is this is the case,
	 * all fields will have the value 0xff. This function takes care of zero initializing
	 * all fields which have 0 as the default value.
	 */
	static ReturnValue_t zeroOutDefaultZeroFields(int* errorField);

protected:
	static std::array<uint8_t, sizeof(CriticalDataBlock)> criticalBlock;

	virtual ReturnValue_t handleMemoryLoad(uint32_t address,
			const uint8_t* data, uint32_t size, uint8_t** dataPointer);
	virtual ReturnValue_t handleMemoryDump(uint32_t address, uint32_t size,
			uint8_t** dataPointer, uint8_t* dumpTarget);
	virtual ReturnValue_t setAddress(uint32_t* startAddress);

	virtual ReturnValue_t initialize() override;

private:
	static void genericBlockPrinter(size_t blockSize);
};

#endif /* SAM9G20_MEMORY_FRAMHANDLER_H_ */
