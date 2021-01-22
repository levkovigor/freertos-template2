#ifndef SAM9G20_MEMORY_FRAMHANDLER_H_
#define SAM9G20_MEMORY_FRAMHANDLER_H_

#include <fsfw/memory/HasMemoryIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>
#include <fsfw/serialize/SerialLinkedListAdapter.h>
#include <fsfw/tasks/ExecutableObjectIF.h>
#include <sam9g20/common/FRAMApi.h>
#include <sam9g20/common/CriticalDataBlock.h>

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
	 * Print the critical block, byte-wise, 4 bytes per line. For debugging purpose,
	 * do not use in mission code. It will later be possible to also dump the critical block
	 * as a binary file.
	 */
	void printCriticalBlock();

protected:
	std::array<uint8_t, sizeof(CriticalDataBlock)> criticalBlock;

	virtual ReturnValue_t handleMemoryLoad(uint32_t address,
			const uint8_t* data, uint32_t size, uint8_t** dataPointer);
	virtual ReturnValue_t handleMemoryDump(uint32_t address, uint32_t size,
			uint8_t** dataPointer, uint8_t* dumpTarget);
	virtual ReturnValue_t setAddress(uint32_t* startAddress);

	virtual ReturnValue_t initialize() override;
};

#endif /* SAM9G20_MEMORY_FRAMHANDLER_H_ */
