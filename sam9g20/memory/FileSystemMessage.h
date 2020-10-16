#ifndef SAM9G20_MEMORY_FILESYSTEMMESSAGE_H_
#define SAM9G20_MEMORY_FILESYSTEMMESSAGE_H_

#include <fsfw/ipc/CommandMessage.h>
#include <fsfw/storagemanager/StorageManagerIF.h>
#include <fsfw/objectmanager/SystemObject.h>

/**
 * @brief   These messages are sent to an object implementing HasFilesystemIF.
 * @author  Jakob Meier, R. Mueller
 */
class FileSystemMessage {
public:
    // Instantiation forbidden
	FileSystemMessage() = delete;

	static const uint8_t MESSAGE_ID = messagetypes::FILE_SYSTEM_MESSAGE;
	/* PUS standard  (ECSS-E-ST-70-41C15 2016 p.654) */
	static const Command_t CMD_CREATE_FILE = MAKE_COMMAND_ID(1);
	static const Command_t CMD_DELETE_FILE = MAKE_COMMAND_ID(2);
	/** Report file attributes */
	static const Command_t CMD_REPORT_FILE_ATTRIBUTES = MAKE_COMMAND_ID(3);
	static const Command_t REPLY_REPORT_FILE_ATTRIBUTES = MAKE_COMMAND_ID(4);
	/** Command to lock a file, setting it read-only */
	static const Command_t CMD_LOCK_FILE = MAKE_COMMAND_ID(5);
	/** Command to unlock a file, enabling further operations on it */
	static const Command_t CMD_UNLOCK_FILE = MAKE_COMMAND_ID(6);
	/**
	 * Find file in repository, using a search pattern.
	 * Please note that * is the wildcard character.
	 * For example, when looking for all files which start with have the
	 * structure tm<something>.bin, tm*.bin can be used.
	 */
	static const Command_t CMD_FIND_FILE = MAKE_COMMAND_ID(7);
	static const Command_t CMD_CREATE_DIRECTORY = MAKE_COMMAND_ID(9);
    static const Command_t CMD_DELETE_DIRECTORY = MAKE_COMMAND_ID(10);
	static const Command_t CMD_RENAME_DIRECTORY = MAKE_COMMAND_ID(11);

	/** Dump contents of a repository */
	static const Command_t CMD_DUMP_REPOSITORY = MAKE_COMMAND_ID(12);
	/** Repository dump reply */
	static const Command_t REPLY_DUMY_REPOSITORY = MAKE_COMMAND_ID(13);

	/** Append operation commands */
    static const Command_t CMD_APPEND_TO_FILE = MAKE_COMMAND_ID(130);
    static const Command_t CMD_FINISH_APPEND_TO_FILE = MAKE_COMMAND_ID(131);
    static const Command_t REPLY_FINISH_APPEND = MAKE_COMMAND_ID(132);

    static const Command_t CMD_READ_FROM_FILE = MAKE_COMMAND_ID(140);
    static const Command_t REPLY_READ_FROM_FILE = MAKE_COMMAND_ID(141);
    static const Command_t CMD_STOP_READ = MAKE_COMMAND_ID(142);
    static const Command_t REPLY_READ_FINISHED_STOP = MAKE_COMMAND_ID(143);

    /** Removes a folder (rm -rf equivalent!). Use with care ! */
    static const Command_t CMD_CLEAR_REPOSITORY = MAKE_COMMAND_ID(180);
    /** Clears the whole SD card. Use with care ! */
    static const Command_t CMD_CLEAR_SD_CARD = MAKE_COMMAND_ID(181);
    /** Formats the SD card (which also clears it!). Use with care ! */
    static const Command_t CMD_FORMAT_SD_CARD = MAKE_COMMAND_ID(182);

	static const Command_t COMPLETION_SUCCESS = MAKE_COMMAND_ID(200);
	static const Command_t COMPLETION_FAILED = MAKE_COMMAND_ID(201);

    static const Command_t NOTIFICATION_CEASE_SD_CARD_OPERATION =
            MAKE_COMMAND_ID(205);

	static void setCreateFileCommand(CommandMessage* message,
	        store_address_t storeId);
	static void setDeleteFileCommand(CommandMessage* message,
	        store_address_t storeId);

	static void setReportFileAttributesCommand(CommandMessage* message,
            store_address_t storeId);
	static void setReportFileAttributesReply(CommandMessage* message,
            store_address_t storeId);

	static void setCreateDirectoryCommand(CommandMessage* message,
	        store_address_t storeId);
	static void setDeleteDirectoryCommand(CommandMessage* message,
	        store_address_t storeId);

    static void setLockFileCommand(CommandMessage* message,
            store_address_t storeId);
    static void setUnlockFileCommand(CommandMessage* message,
            store_address_t storeId);

	static void setWriteCommand(CommandMessage* message,
	        store_address_t storeId);
	static void setFinishStopWriteCommand(CommandMessage* message,
	        store_address_t storeId);
	static void setFinishStopWriteReply(CommandMessage* message,
			store_address_t storeId);

	static void setClearSdCardCommand(CommandMessage* message);
	static void setFormatSdCardCommand(CommandMessage* message);


    static void setReadCommand(CommandMessage* message,
            store_address_t storeId);
    static void setReadFinishedReply(CommandMessage* message,
            store_address_t storeId);
	static void setReadReply(CommandMessage* message, bool readFinished,
	        store_address_t storeId);
	static bool getReadReply(const CommandMessage* message,
	        store_address_t* storeId);

	static void setFinishAppendReply(CommandMessage* message,
			store_address_t storeId);
    static void setSuccessReply(CommandMessage* message);
    static void setFailureReply(CommandMessage* message,
            ReturnValue_t errorCode, uint32_t errorParam = 0);

	static store_address_t getStoreId(const CommandMessage* message);
	static ReturnValue_t getFailureReply(const CommandMessage* message,
			uint32_t* errorParam = nullptr);

	static void setCeaseSdCardOperationNotification( CommandMessage* command);
};

#endif /* SAM9G20_MEMORY_FILESYSTEMMESSAGE_H_ */
