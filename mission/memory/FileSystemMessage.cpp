#include "FileSystemMessage.h"

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
