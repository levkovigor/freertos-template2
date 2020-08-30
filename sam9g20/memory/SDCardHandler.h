/**
 * \file SDCardHandler.h
 *
 * \date 27.10.2019
 */

#ifndef SAM9G20_MEMORY_SDCARDHANDLER_H_
#define SAM9G20_MEMORY_SDCARDHANDLER_H_

#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/memory/AcceptsMemoryMessagesIF.h>
#include <fsfw/ipc/MessageQueueIF.h>

/**
 * Additional abstraction layer to encapsulate access to SD cards
 * using the iOBC HCC FAT API.
 */
class SDCardHandler : public SystemObject, public ExecutableObjectIF, public AcceptsMemoryMessagesIF {
public:
	SDCardHandler(object_id_t objectId_);
	virtual ~SDCardHandler();

	ReturnValue_t handleMemoryLoad(uint32_t address, const uint8_t* data, uint32_t size, uint8_t** dataPointer);
	ReturnValue_t handleMemoryDump(uint32_t address, uint32_t size, uint8_t** dataPointer, uint8_t* dumpTarget);

	MessageQueueId_t getCommandQueue() const;

	ReturnValue_t performOperation(uint8_t operationCode = 0);

	ReturnValue_t setAddress( uint32_t* startAddress );

private:
	/**
	 * The MessageQueue used to receive commands, data and to send replies.
	 */
	MessageQueueIF* commandQueue;

	uint32_t queueDepth = 20;

	StorageManagerIF *IPCStore;
};

#endif /* SAM9G20_MEMORY_SDCARDHANDLER_H_ */
