#ifndef MISSION_MEMORY_FRAMHANDLER_H_
#define MISSION_MEMORY_FRAMHANDLER_H_

#include <fsfw/memory/HasMemoryIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>
#include <fsfw/serialize/SerialLinkedListAdapter.h>
#include <fsfw/tasks/ExecutableObjectIF.h>
#include <sam9g20/common/FRAMApi.h>


/**
 * Additional abstraction layer to encapsulate access to FRAM
 */
class FRAMHandler : public SystemObject,
        public HasMemoryIF,
        public HasReturnvaluesIF {
public:
	FRAMHandler(object_id_t objectId);
	virtual~ FRAMHandler();

protected:

	virtual ReturnValue_t handleMemoryLoad(uint32_t address,
			const uint8_t* data, uint32_t size, uint8_t** dataPointer);
	virtual ReturnValue_t handleMemoryDump(uint32_t address, uint32_t size,
			uint8_t** dataPointer, uint8_t* dumpTarget);
	virtual ReturnValue_t setAddress(uint32_t* startAddress);

	virtual ReturnValue_t initialize() override;
};

#endif /* MISSION_MEMORY_FRAMHANDLER_H_ */
