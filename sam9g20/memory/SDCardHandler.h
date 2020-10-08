#ifndef SAM9G20_MEMORY_SDCARDHANDLER_H_
#define SAM9G20_MEMORY_SDCARDHANDLER_H_

#include "SDCardDefinitions.h"
#include <subsystemIdRanges.h>

#include <fsfw/action/HasActionsIF.h>
#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/memory/AcceptsMemoryMessagesIF.h>
#include <fsfw/ipc/MessageQueueIF.h>
#include <fsfw/memory/HasFileSystemIF.h>
#include <sam9g20/memory/SDCardApi.h>


class PeriodicTaskIF;
class Countdown;
class ReadCommand;

/**
 * Additional abstraction layer to encapsulate access to SD cards
 * using the iOBC HCC FAT API.
 */
class SDCardHandler : public SystemObject,
        public ExecutableObjectIF,
        public HasFileSystemIF,
        public HasActionsIF {
    friend class SDCardAccess;
public:
    // todo: make these configurable via OBSWConfig.h
    static constexpr uint8_t MAX_FILE_MESSAGES_HANDLED_PER_CYCLE = 10;
    static constexpr uint32_t MAX_MESSAGE_QUEUE_DEPTH = 20;


    static constexpr uint8_t INTERFACE_ID = CLASS_ID::SD_CARD_HANDLER;

    static const uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::SD_CARD_HANDLER;


    static constexpr Event SD_CARD_SWITCHED = MAKE_EVENT(0x00, SEVERITY::MEDIUM); //!< It was not possible to open the preferred SD card so the other was used. P1: Active volume
    static constexpr Event SD_CARD_ACCESS_FAILED = MAKE_EVENT(0x01, SEVERITY::HIGH); //!< Opening failed for both SD cards.
    static constexpr Event SEQUENCE_PACKET_MISSING_EVENT = MAKE_EVENT(0x02, SEVERITY::LOW); //!< P1: Sequence packet missing.

    /** Service 8 Commands */

    //! [EXPORT] : [COMMAND] Dump structure of whole SD card as ASCII file.
    static constexpr ActionId_t DUMP_FILE_STRUCTURE = 1;
    //! [EXPORT] : [COMMAND] Print SD card to console.
    static constexpr ActionId_t PRINT_SD_CARD = 2;

    //! [EXPORT] : [COMMAND] Swap the active SD card.
    static constexpr ActionId_t SELECT_ACTIVE_SD_CARD = 5;
    static constexpr ActionId_t REPORT_ACTIVE_SD_CARD = 6;
    static constexpr ActionId_t SELECT_PREFERED_SD_CARD = 10;
    static constexpr ActionId_t REPORT_PREFERED_SD_CARD = 11;

    //! [EXPORT] : [COMMAND] Clears SD card. Use with care!
    static constexpr ActionId_t CLEAR_SD_CARD = 20;
    //! [EXPORT] : [COMMAND] Formats SD card which also deletes everything.
    //! Use with care!
    static constexpr ActionId_t FORMAT_SD_CARD = 21;

    SDCardHandler(object_id_t objectId_);
    virtual ~SDCardHandler();

    MessageQueueId_t getCommandQueue() const override;
    ReturnValue_t performOperation(uint8_t operationCode = 0) override;
    ReturnValue_t initialize() override;
    ReturnValue_t initializeAfterTaskCreation() override;
    void setTaskIF(PeriodicTaskIF* executingTask) override;

    /** HasActionIF */
    ReturnValue_t executeAction(ActionId_t actionId,
        MessageQueueId_t commandedBy, const uint8_t* data,
        size_t size) override;

    // Useful functions for development
    static ReturnValue_t printRepository(const char* repository);
    static ReturnValue_t printSdCard();

    // This function will dump the current SD card format in ASCII format
    static ReturnValue_t dumpSdCard();
private:
    /**
     * The MessageQueue used to receive commands, data and to send replies.
     */
    MessageQueueIF* commandQueue;
    ActionHelper actionHelper;
    Countdown* countdown;

    PeriodicTaskIF* executingTask = nullptr;
    dur_millis_t periodMs = 0;

    StorageManagerIF *IPCStore;

    bool fileSystemWasUsedOnce = false;
    size_t currentReadPos = 0;
    bool fileSystemOpen = false;

    ReturnValue_t getStoreData(store_address_t& storeId,
            ConstStorageAccessor& accessor,
            const uint8_t** ptr, size_t* size);

    // will also be moved to FRAM!
    VolumeId preferedVolume = SD_CARD_0;
    VolumeId activeVolume = SD_CARD_0;

    VolumeId determineVolumeToOpen();
    ReturnValue_t handleAccessResult(ReturnValue_t accessResult);
    ReturnValue_t handleMultipleMessages(CommandMessage* message);

    static ReturnValue_t printHelper(uint8_t recursionDepth);

    // Right now, only supports one file upload at a time.
    static constexpr uint16_t UNSET_SEQUENCE = 0xffff;
    uint16_t lastPacketNumber = UNSET_SEQUENCE;

    ReturnValue_t handleMessage(CommandMessage* message);
    ReturnValue_t handleFileMessage(CommandMessage* message);

    /** HasFilesystemIF overrides */
    ReturnValue_t createFile(const char* repositoryPath, const char* filename,
            const uint8_t* data, size_t size, void* args = nullptr) override;
    ReturnValue_t appendToFile(const char* repositoryPath, const char* filename,
            const uint8_t* data, size_t size, uint16_t packetNumber,
            void* args = nullptr) override;
    ReturnValue_t deleteFile(const char* repositoryPath,
            const char* filename, void* args = nullptr) override;

    ReturnValue_t createDirectory(const char* repositoryPath,
            const char* dirname);
    ReturnValue_t deleteDirectory(const char* repositoryPath,
            const char* dirname);
    ReturnValue_t changeDirectory(const char* repositoryPath);

    ReturnValue_t handleCreateFileCommand(CommandMessage* message);
    ReturnValue_t handleDeleteFileCommand(CommandMessage* message);

    ReturnValue_t handleReportAttributesCommand(CommandMessage* message);
    ReturnValue_t handleCreateDirectoryCommand(CommandMessage* message);
    ReturnValue_t handleDeleteDirectoryCommand(CommandMessage* message);

    ReturnValue_t handleLockFileCommand(CommandMessage* message, bool lock);

    ReturnValue_t handleAppendCommand(CommandMessage* message);
    ReturnValue_t handleFinishAppendCommand(CommandMessage* message);

    ReturnValue_t handleReadCommand(CommandMessage* message);
    ReturnValue_t openFileForReading(const char* repositoryPath,
    		const char* filename, F_FILE** file,
    		size_t readPosition, size_t* fileSize);
    ReturnValue_t handleReadReply(ReadCommand& command);

    void sendCompletionReply(bool success = true,
            ReturnValue_t errorCode = HasReturnvaluesIF::RETURN_OK,
			uint32_t errorParam = 0);
    ReturnValue_t generateFinishAppendReply(RepositoryPath* repoPath,
    		FileName* fileName, size_t filesize, bool locked);



};

#endif /* SAM9G20_MEMORY_SDCARDHANDLER_H_ */
