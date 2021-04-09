#include "Service6MemoryManagement.h"

#include <fsfw/memory/AcceptsMemoryMessagesIF.h>
#include <fsfw/globalfunctions/CRC.h>
#include <fsfw/tmtcpacket/pus/TmPacketStored.h>
#include <mission/pus/servicepackets/Service6Packets.h>

Service6MemoryManagement::Service6MemoryManagement(object_id_t objectId,
        uint16_t apid, uint8_t serviceId):
	CommandingServiceBase(objectId, apid, serviceId,
			NUM_PARALLEL_COMMANDS, COMMAND_TIMEOUT_SECONDS) {}

Service6MemoryManagement::~Service6MemoryManagement() {}

ReturnValue_t Service6MemoryManagement::isValidSubservice(uint8_t subservice) {
    switch(static_cast<Subservice>(subservice)) {
    case Subservice::LOAD_DATA_TO_MEMORY:
    case Subservice::DUMP_MEMORY:
    case Subservice::CHECK_MEMORY:
        return HasReturnvaluesIF::RETURN_OK;
    default:
        return HasReturnvaluesIF::RETURN_FAILED;
    }
}

ReturnValue_t Service6MemoryManagement::getMessageQueueAndObject(
        uint8_t subservice, const uint8_t *tcData, size_t tcDataLen,
        MessageQueueId_t *id, object_id_t *objectId) {
    if(tcDataLen < sizeof(object_id_t)) {
        return CommandingServiceBase::INVALID_TC;
    }
    SerializeAdapter::deSerialize(objectId, &tcData, &tcDataLen,
            SerializeIF::Endianness::BIG);

    return checkInterfaceAndAcquireMessageQueue(id,objectId);
}

ReturnValue_t Service6MemoryManagement::checkInterfaceAndAcquireMessageQueue(
        MessageQueueId_t* messageQueueToSet, object_id_t* objectId) {
    AcceptsMemoryMessagesIF * destination = objectManager->
            get<AcceptsMemoryMessagesIF>(*objectId);
    if(destination == nullptr) {
        return CommandingServiceBase::INVALID_OBJECT;
    }

    *messageQueueToSet = destination->getCommandQueue();
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Service6MemoryManagement::prepareCommand(
		CommandMessage* message, uint8_t subservice, const uint8_t* tcData,
		size_t tcDataLen, uint32_t* state, object_id_t objectId) {
	switch(static_cast<Subservice>(subservice)) {
	case Subservice::LOAD_DATA_TO_MEMORY:
		return prepareMemoryLoadCommand(message, tcData, tcDataLen);
	case Subservice::DUMP_MEMORY:
		return prepareMemoryDumpCommand(message, tcData, tcDataLen);
	case Subservice::CHECK_MEMORY:
		return prepareMemoryCheckCommand(message, tcData, tcDataLen);
	default:
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

ReturnValue_t Service6MemoryManagement::prepareMemoryLoadCommand(
		CommandMessage* message, const uint8_t * tcData, size_t size) {
    // Todo: make max data size configurable
	LoadToMemoryCommand<1024> loadCommand;
	/* Copy TC application data into LoadToMemoryCommand instance by using a
	deserialization adapter. This can be used to easily access application
	data */
	ReturnValue_t result = loadCommand.deSerialize(&tcData,&size,
	        SerializeIF::Endianness::BIG);
	if(result != HasReturnvaluesIF::RETURN_OK){
		return result;
	}

	// store data to be loaded into the Inter Process Communication Store
	store_address_t storeAddress;
	result = IPCStore->addData(&storeAddress,loadCommand.getData(),
			loadCommand.getDataSize());
	if(result != HasReturnvaluesIF::RETURN_OK)  {
		return result;
	}

	MemoryMessage::setMemoryLoadCommand(message, loadCommand.getAddress(),
			storeAddress);
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Service6MemoryManagement::prepareMemoryDumpCommand(
		CommandMessage *message, const uint8_t *tcData, size_t size) {
	DumpMemoryCommand dumpCommand;
	ReturnValue_t result = dumpCommand.deSerialize(&tcData,&size,
	        SerializeIF::Endianness::BIG);
	if(result != HasReturnvaluesIF::RETURN_OK)  {
		return result;
	}

	MemoryMessage::setMemoryDumpCommand(message, dumpCommand.getAddress(),
			dumpCommand.getDumpLength());
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Service6MemoryManagement::prepareMemoryCheckCommand(
		CommandMessage *message, const uint8_t *tcData, size_t size) {
	CheckMemoryCommand command;
	ReturnValue_t result = command.deSerialize(&tcData, &size,
	        SerializeIF::Endianness::BIG);
	if(result != HasReturnvaluesIF::RETURN_OK)  {
		return result;
	}

	MemoryMessage::setMemoryCheckCommand(message, command.getAddress(),
			command.getCheckLength());
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Service6MemoryManagement::handleReply(
		const CommandMessage* reply, Command_t previousCommand,
		uint32_t* state, CommandMessage* optionalNextCommand,
		object_id_t objectId, bool* isStep) {
    Command_t reply_id = reply->getCommand();
    switch(reply_id) {
    case MemoryMessage::REPLY_MEMORY_DUMP:
        return sendMemoryDumpPacket(objectId,reply);

    case MemoryMessage::REPLY_MEMORY_CHECK:
        return sendMemoryCheckPacket(reply);

    case MemoryMessage::REPLY_MEMORY_FAILED:
    default:
        return MemoryMessage::getErrorCode(reply);
    }
}

ReturnValue_t Service6MemoryManagement::sendMemoryDumpPacket(object_id_t objectId,
		const CommandMessage *reply) {
	const uint8_t* dataToBeDumped = nullptr;
	size_t sizeOfData = 0;
	ReturnValue_t result = IPCStore->getData(MemoryMessage::getStoreID(reply),
			&dataToBeDumped,&sizeOfData);
	if(result != HasReturnvaluesIF::RETURN_OK){
		return result;
	}
	uint16_t crc16 = CRC::crc16ccitt(dataToBeDumped,sizeOfData);
	// Initialize and build dumpPacket
	MemoryDumpPacket dumpPacket(objectId,MemoryMessage::getAddress(reply),
			sizeOfData, dataToBeDumped, crc16);
	// todo: does not have returnvalue yet. Annoy Steffen :-)
	sendTmPacket(static_cast<uint8_t>(Subservice::MEMORY_DUMP_REPORT),
			&dumpPacket, nullptr);
	result = IPCStore->deleteData(MemoryMessage::getStoreID(reply));
	if(result != RETURN_OK){
		return result;
	}
	return CommandingServiceBase::EXECUTION_COMPLETE;
}

ReturnValue_t Service6MemoryManagement::sendMemoryCheckPacket(
		const CommandMessage *reply) {
	MemoryCheckPacket checkPacket;
	// serializing done by packet definition
	checkPacket.setAddress(MemoryMessage::getAddress(reply));
	checkPacket.setLength(MemoryMessage::getLength(reply));
	checkPacket.setChecksum(MemoryMessage::getCrc(reply));

	TmPacketStored tmPacket(apid, service,
			static_cast<uint8_t>(Subservice::MEMORY_CHECK_REPORT),
			packetSubCounter++, &checkPacket);
	sendTmPacket(static_cast<uint8_t>(Subservice::MEMORY_CHECK_REPORT),
			&checkPacket);
	return CommandingServiceBase::EXECUTION_COMPLETE;
}


