#ifndef MISSION_MEMORY_FRAMHANDLER_H_
#define MISSION_MEMORY_FRAMHANDLER_H_

#include <fsfw/memory/HasMemoryIF.h>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>
#include <fsfw/serialize/SerialLinkedListAdapter.h>
#include <sam9g20/common/FRAMApi.h>

/**
 * This class will gather all critical data which is written to FRAM
 */
class FRAMCriticalDataClass: public SerialLinkedListAdapter<SerializeIF> {
public:
	FRAMCriticalDataClass() {
		setLinks();
	}
private:
	void setLinks() {

	}

	SerializeElement<uint8_t> rebootCounter;
	SerializeElement<size_t> norFlashBinarySize;

};

/**
 * Additional abstraction layer to encapsulate access to FRAM
 */
class FRAMHandler : public HasMemoryIF, public HasReturnvaluesIF {
public:

	FRAMHandler();
	virtual~ FRAMHandler();

protected:
	virtual ReturnValue_t handleMemoryLoad(uint32_t address,
			const uint8_t* data, uint32_t size, uint8_t** dataPointer);
	virtual ReturnValue_t handleMemoryDump(uint32_t address, uint32_t size,
			uint8_t** dataPointer, uint8_t* dumpTarget);
	virtual ReturnValue_t setAddress( uint32_t* startAddress );
};

#endif /* MISSION_MEMORY_FRAMHANDLER_H_ */
