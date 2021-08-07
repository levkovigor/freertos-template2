#include "FileSystemMessage.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/storagemanager/StorageManagerIF.h"

void FileSystemMessage::setClearSdCardCommand(CommandMessage *message) {
    message->setCommand(CMD_CLEAR_SD_CARD);
}

void FileSystemMessage::setFormatSdCardCommand(CommandMessage *message) {
    message->setCommand(CMD_FORMAT_SD_CARD);
}

void FileSystemMessage::setCeaseSdCardOperationNotification(
        CommandMessage *command) {
    command->setCommand(NOTIFICATION_CEASE_SD_CARD_OPERATION);
}

ReturnValue_t FileSystemMessage::clear(CommandMessage *message) {
	switch(message->getCommand()) {
	case(CMD_CLEAR_REPOSITORY): {
		store_address_t storeId = GenericFileSystemMessage::getStoreId(message);
		auto ipcStore = ObjectManager::instance()->get<StorageManagerIF>(objects::IPC_STORE);
		if(ipcStore == nullptr) {
			return HasReturnvaluesIF::RETURN_FAILED;
		}
		return ipcStore->deleteData(storeId);
	}
	default: {
		break;
	}
	}
	return GenericFileSystemMessage::clear(message);
}
