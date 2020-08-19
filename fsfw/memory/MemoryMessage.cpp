#include "../memory/MemoryMessage.h"
#include "../objectmanager/ObjectManagerIF.h"
MemoryMessage::MemoryMessage() {
}

uint32_t MemoryMessage::getAddress(const CommandMessage* message) {
	return message->getParameter();
}

store_address_t MemoryMessage::getStoreID(const CommandMessage* message) {
	store_address_t temp;
	temp.raw = message->getParameter2();
	return temp;
}

uint32_t MemoryMessage::getLength(const CommandMessage* message) {
	return message->getParameter2();
}

ReturnValue_t MemoryMessage::setMemoryDumpCommand(CommandMessage* message,
		uint32_t address, uint32_t length) {
	message->setCommand(CMD_MEMORY_DUMP);
	message->setParameter( address );
	message->setParameter2( length );
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t MemoryMessage::setMemoryDumpReply(CommandMessage* message, store_address_t storageID) {
	message->setCommand(REPLY_MEMORY_DUMP);
	message->setParameter2( storageID.raw );
	return HasReturnvaluesIF::RETURN_OK;
}

void MemoryMessage::setMemoryLoadCommand(CommandMessage* message,
		uint32_t address, store_address_t storageID) {
	message->setCommand(CMD_MEMORY_LOAD);
	message->setParameter( address );
	message->setParameter2( storageID.raw );
}

ReturnValue_t MemoryMessage::getErrorCode(const CommandMessage* message) {
	return message->getParameter();
}

void MemoryMessage::clear(CommandMessage* message) {
	switch (message->getCommand()) {
	case CMD_MEMORY_LOAD:
	case REPLY_MEMORY_DUMP: {
		StorageManagerIF *ipcStore = objectManager->get<StorageManagerIF>(
				objects::IPC_STORE);
		if (ipcStore != NULL) {
			ipcStore->deleteData(getStoreID(message));
		}
	}
	/* NO BREAK falls through*/
	case CMD_MEMORY_DUMP:
	case CMD_MEMORY_CHECK:
	case REPLY_MEMORY_CHECK:
	case END_OF_MEMORY_COPY:
		message->setCommand(CommandMessage::CMD_NONE);
		message->setParameter(0);
		message->setParameter2(0);
		break;
		}
}

ReturnValue_t MemoryMessage::setMemoryCheckCommand(CommandMessage* message,
		uint32_t address, uint32_t length) {
	message->setCommand(CMD_MEMORY_CHECK);
	message->setParameter( address );
	message->setParameter2( length );
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t MemoryMessage::setMemoryCheckReply(CommandMessage* message,
		uint16_t crc) {
	message->setCommand(REPLY_MEMORY_CHECK);
	message->setParameter( crc );
	return HasReturnvaluesIF::RETURN_OK;
}

void MemoryMessage::setCrcReturnValue(CommandMessage* message, ReturnValue_t returnValue){
	message->setParameter(returnValue<<16);
};

uint16_t MemoryMessage::getCrc(const CommandMessage* message) {
	return (uint16_t)(message->getParameter());
}

ReturnValue_t MemoryMessage::getCrcReturnValue(const CommandMessage* message){
	return (message->getParameter()>>16);
}

Command_t MemoryMessage::getInitialCommand(const CommandMessage* message) {
	return message->getParameter2();
}

ReturnValue_t MemoryMessage::setMemoryReplyFailed(CommandMessage* message,
		ReturnValue_t errorCode, Command_t initialCommand) {
	message->setCommand(REPLY_MEMORY_FAILED);
	message->setParameter(errorCode);
	message->setParameter2(initialCommand);
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t MemoryMessage::setMemoryCopyEnd(CommandMessage* message) {
	message->setCommand(END_OF_MEMORY_COPY);
	message->setParameter(0);
	message->setParameter2(0);
	return HasReturnvaluesIF::RETURN_OK;
}

