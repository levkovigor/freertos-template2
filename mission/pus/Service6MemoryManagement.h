#ifndef MISSION_PUS_SERVICE6MEMORYMANAGEMENT_H_
#define MISSION_PUS_SERVICE6MEMORYMANAGEMENT_H_

#include <fsfw/tmtcservices/CommandingServiceBase.h>

/**
 * @brief Load to memory, dump memory, check memory
 *
 * Full Documentation: ECSS-E-ST-70-41C or ECSS-E-70-41A
 * Dissertation Baetz p. 115, 116, 165-167
 *
 * This service provides the capability to manage memory by either
 * loading data into a specified memory address, dumping memory or calculating the
 * checksum of a memory area.
 *
 * This is a gateway service. It relays device commands using the software bus.
 * This service is very closely tied to the CommandingServiceBase template class.
 * There is constant interaction between this Service Base und a
 * subclass like this service. There are 4 adaption points (abstract functions)
 * for component implementation through the Commanding Service Base.

 * @ingroup pus_services
 */
class Service6MemoryManagement : public CommandingServiceBase {
public:
	static constexpr uint8_t NUM_PARALLEL_COMMANDS = 4;
	static constexpr uint16_t COMMAND_TIMEOUT_SECONDS = 60;

	Service6MemoryManagement(object_id_t objectId, uint16_t apid,
	        uint8_t serviceId);
	virtual ~Service6MemoryManagement();
protected:
	//! CommandingServiceBase (CSB) abstract functions. See CSB documentation.
	ReturnValue_t isValidSubservice(uint8_t subservice) override;
	ReturnValue_t getMessageQueueAndObject(uint8_t subservice,
			const uint8_t *tcData, size_t tcDataLen, MessageQueueId_t *id,
			object_id_t *objectId) override;
	ReturnValue_t prepareCommand(CommandMessage* message,uint8_t subservice,
	        const uint8_t *tcData, size_t tcDataLen, uint32_t *state,
	        object_id_t objectId) override;
	ReturnValue_t handleReply(const CommandMessage* reply,
			Command_t previousCommand, uint32_t *state,
			CommandMessage* optionalNextCommand, object_id_t objectId,
			bool *isStep) override;

private:
	uint16_t packetSubCounter = 0;

	ReturnValue_t checkInterfaceAndAcquireMessageQueue(
	        MessageQueueId_t* messageQueueToSet, object_id_t* objectId);

	ReturnValue_t prepareMemoryLoadCommand(CommandMessage* message,
			const uint8_t * tcData, size_t size);
	ReturnValue_t prepareMemoryDumpCommand(CommandMessage* message,
			const uint8_t * tcData, size_t size);
	ReturnValue_t prepareMemoryCheckCommand(CommandMessage* message,
			const uint8_t * tcData, size_t size);
	ReturnValue_t sendMemoryDumpPacket(object_id_t objectId,
			const CommandMessage* reply);
	ReturnValue_t sendMemoryCheckPacket(const CommandMessage* reply);

	enum class Subservice: uint8_t {
		LOAD_DATA_TO_MEMORY = 2, //!< [EXPORT] : [COMMAND] Load data to target memory
		DUMP_MEMORY = 5, //!< [EXPORT] : [COMMAND] Dump data at target memory
		MEMORY_DUMP_REPORT = 6, //!< [EXPORT] : [REPLY] Dumped memory reply
		CHECK_MEMORY = 9, //!< [EXPORT] : [COMMAND] Check target memory checksum
		MEMORY_CHECK_REPORT = 10, //!< [EXPORT] : [REPLY] Memory check reply
	};
};




#endif /* MISSION_PUS_SERVICE6MEMORYMANAGEMENT_H_ */
