#include "FileSystemMessage.h"

#include <fsfw/objectmanager/ObjectManagerIF.h>

void FileSystemMessage::setCreateFileCommand(CommandMessage* message,
        store_address_t storeId) {
    message->setCommand(CMD_CREATE_FILE);
    message->setParameter2(storeId.raw);
}

void FileSystemMessage::setDeleteFileCommand(
        CommandMessage* message, store_address_t storeId) {
    message->setCommand(CMD_DELETE_FILE);
    message->setParameter2(storeId.raw);
}

void FileSystemMessage::setCreateDirectoryCommand(
        CommandMessage* message, store_address_t storeId) {
    message->setCommand(CMD_CREATE_DIRECTORY);
    message->setParameter2(storeId.raw);
}

void FileSystemMessage::setReportFileAttributesCommand(CommandMessage *message,
        store_address_t storeId) {
    message->setCommand(CMD_REPORT_FILE_ATTRIBUTES);
    message->setParameter2(storeId.raw);
}

void FileSystemMessage::setReportFileAttributesReply(CommandMessage *message,
        store_address_t storeId) {
    message->setCommand(REPLY_REPORT_FILE_ATTRIBUTES);
    message->setParameter2(storeId.raw);
}

void FileSystemMessage::setLockFileCommand(CommandMessage *message,
        store_address_t storeId) {
    message->setCommand(CMD_LOCK_FILE);
    message->setParameter2(storeId.raw);
}

void FileSystemMessage::setUnlockFileCommand(CommandMessage *message,
        store_address_t storeId) {
    message->setCommand(CMD_UNLOCK_FILE);
    message->setParameter2(storeId.raw);
}

void FileSystemMessage::setDeleteDirectoryCommand(CommandMessage* message,
        store_address_t storeId) {
    message->setCommand(CMD_DELETE_DIRECTORY);
    message->setParameter2(storeId.raw);
}

void FileSystemMessage::setCopyCommand(CommandMessage* message,
        store_address_t storeId) {
    message->setCommand(CMD_COPY_FILE);
    message->setParameter2(storeId.raw);
}

void FileSystemMessage::setWriteCommand(CommandMessage* message,
        store_address_t storeId) {
    message->setCommand(CMD_APPEND_TO_FILE);
    message->setParameter2(storeId.raw);
}

void FileSystemMessage::setReadCommand(CommandMessage* message,
        store_address_t storeId) {
    message->setCommand(CMD_READ_FROM_FILE);
    message->setParameter2(storeId.raw);
}

void FileSystemMessage::setFinishAppendReply(CommandMessage* message,
        store_address_t storageID) {
    message->setCommand(REPLY_FINISH_APPEND);
    message->setParameter2(storageID.raw);
}

void FileSystemMessage::setReadReply(CommandMessage* message,
        bool readFinished, store_address_t storeId) {
    message->setCommand(REPLY_READ_FROM_FILE);
    message->setParameter(readFinished);
    message->setParameter2(storeId.raw);
}

void FileSystemMessage::setReadFinishedReply(CommandMessage *message,
        store_address_t storeId) {
    message->setCommand(REPLY_READ_FINISHED_STOP);
    message->setParameter2(storeId.raw);
}

bool FileSystemMessage::getReadReply(const CommandMessage *message,
        store_address_t *storeId) {
    if(storeId != nullptr) {
        (*storeId).raw = message->getParameter2();
    }
    return message->getParameter();
}

void FileSystemMessage::setSuccessReply(CommandMessage *message) {
    message->setCommand(COMPLETION_SUCCESS);
}

void FileSystemMessage::setClearSdCardCommand(CommandMessage *message) {
    message->setCommand(CMD_CLEAR_SD_CARD);
}

void FileSystemMessage::setFormatSdCardCommand(CommandMessage *message) {
    message->setCommand(CMD_FORMAT_SD_CARD);
}


void FileSystemMessage::setFinishStopWriteCommand(CommandMessage *message,
        store_address_t storeId) {
    message->setCommand(CMD_FINISH_APPEND_TO_FILE);
    message->setParameter2(storeId.raw);
}

void FileSystemMessage::setFinishStopWriteReply(CommandMessage *message,
        store_address_t storeId) {
    message->setCommand(REPLY_FINISH_APPEND);
    message->setParameter2(storeId.raw);
}

void FileSystemMessage::setFailureReply(CommandMessage *message,
        ReturnValue_t errorCode, uint32_t errorParam) {
    message->setCommand(COMPLETION_FAILED);
    message->setParameter(errorCode);
    message->setParameter2(errorParam);
}

store_address_t FileSystemMessage::getStoreId(const CommandMessage* message) {
    store_address_t temp;
    temp.raw = message->getParameter2();
    return temp;
}

ReturnValue_t FileSystemMessage::getFailureReply(
        const CommandMessage *message, uint32_t* errorParam) {
    if(errorParam != nullptr) {
        *errorParam = message->getParameter2();
    }
    return message->getParameter();
}

void FileSystemMessage::setCeaseSdCardOperationNotification(
        CommandMessage *command) {
    command->setCommand(NOTIFICATION_CEASE_SD_CARD_OPERATION);
}
