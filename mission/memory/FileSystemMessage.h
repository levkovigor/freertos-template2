#ifndef SAM9G20_MEMORY_FILESYSTEMMESSAGE_H_
#define SAM9G20_MEMORY_FILESYSTEMMESSAGE_H_

#include "fsfw/memory/GenericFileSystemMessage.h"


class FileSystemMessage: public GenericFileSystemMessage {
public:
    /* Instantiation forbidden */
    FileSystemMessage() = delete;

    /** Removes a folder (rm -rf equivalent!). Use with care ! */
    static const Command_t CMD_CLEAR_REPOSITORY = MAKE_COMMAND_ID(180);
    /** Clears the whole SD card. Use with care ! */
    static const Command_t CMD_CLEAR_SD_CARD = MAKE_COMMAND_ID(181);
    /** Formats the SD card (which also clears it!). Use with care ! */
    static const Command_t CMD_FORMAT_SD_CARD = MAKE_COMMAND_ID(182);

    static const Command_t NOTIFICATION_CEASE_SD_CARD_OPERATION =
            MAKE_COMMAND_ID(205);

    static void setClearSdCardCommand(CommandMessage* message);
    static void setFormatSdCardCommand(CommandMessage* message);
    static void setCeaseSdCardOperationNotification( CommandMessage* command);
};

#endif /* SAM9G20_MEMORY_FILESYSTEMMESSAGE_H_ */
