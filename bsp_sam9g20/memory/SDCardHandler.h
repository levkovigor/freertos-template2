#ifndef SAM9G20_MEMORY_SDCARDHANDLER_H_
#define SAM9G20_MEMORY_SDCARDHANDLER_H_

#include "OBSWConfig.h"
#include "sdcardDefinitions.h"
#include <events/subsystemIdRanges.h>

#include <bsp_sam9g20/common/SDCardApi.h>
#include <bsp_sam9g20/memory/SDCHStateMachine.h>

#include <fsfw/action/HasActionsIF.h>
#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/memory/AcceptsMemoryMessagesIF.h>
#include <fsfw/ipc/MessageQueueIF.h>
#include <fsfw/memory/HasFileSystemIF.h>
#include <fsfw/timemanager/Countdown.h>

#include <vector>

class SDCardAccess;
class PeriodicTaskIF;
class ReadCommand;

/**
 * @brief   This is the primary handler for access to the SD cards on the iOBC or the AT91
 *          development board.
 * @details
 * It is capable of processing write and read requests from ground or from other software
 * components. Other software components can still use the SD cards directly, but should register
 * at the handler for SD card notifications so that a clean change of the active SD card can be
 * implemented. Right now, this is not enforced strictly, but the SD card access helper will deny
 * any requests to open the file system if a change of active SD card is ongoing.
 */
class SDCardHandler :
        public SystemObject,
        public ExecutableObjectIF,
        public HasFileSystemIF,
        public HasActionsIF {
    friend class SDCardAccess;
    friend class SDCHStateMachine;
public:
    /** Constructor initializes the handler as a system object */
    SDCardHandler(object_id_t objectId);
    virtual ~SDCardHandler();

    static constexpr uint32_t MAX_MESSAGE_QUEUE_DEPTH =
            config::SD_CARD_MQ_DEPTH;
    static constexpr size_t MAX_READ_LENGTH = config::SD_CARD_MAX_READ_LENGTH;

    /** Service 8 Commands */

    //! [EXPORT] : [COMMAND] Dump structure of whole SD card as ASCII file.
    static constexpr ActionId_t DUMP_FILE_STRUCTURE = 1;
    //! [EXPORT] : [COMMAND] Print SD card to console.
    static constexpr ActionId_t PRINT_SD_CARD = 2;

    //! [EXPORT] : [COMMAND] Swap the active SD card.
    static constexpr ActionId_t SELECT_ACTIVE_SD_CARD = 5;
    static constexpr ActionId_t REPORT_ACTIVE_SD_CARD = 6;
    static constexpr ActionId_t SELECT_PREFERRED_SD_CARD = 10;
    static constexpr ActionId_t REPORT_PREFERRED_SD_CARD = 11;
    static constexpr ActionId_t SET_LOAD_OBSW_UPDATE = 12;
    static constexpr ActionId_t GET_LOAD_OBSW_UPDATE = 13;

    //! [EXPORT] : [COMMAND] Clears SD card. Use with care!
    static constexpr ActionId_t CLEAR_SD_CARD = 20;
    //! [EXPORT] : [COMMAND] Formats SD card which also deletes everything.
    //! Use with care!
    static constexpr ActionId_t FORMAT_SD_CARD = 21;

    static constexpr ActionId_t CANCEL_SDCH_OPERATIONS = 30;

    MessageQueueId_t getCommandQueue() const override;

    /** SystemObjectIF */
    ReturnValue_t initialize() override;

    /** ExecutableObjectIF */
    ReturnValue_t performOperation(uint8_t operationCode = 0) override;
    ReturnValue_t initializeAfterTaskCreation() override;
    void setTaskIF(PeriodicTaskIF* executingTask) override;

    /** HasActionIF */
    ReturnValue_t executeAction(ActionId_t actionId,
            MessageQueueId_t commandedBy, const uint8_t* data,
            size_t size) override;

#ifdef ISIS_OBC_G20
    /**
     * Other software components can use the SD card as well without using the SD card handler as
     * the HCC library can process the requests of multiple tasks.
     * However, only one SD card can be active at a time. Therefore, all tasks which use the
     * SD card directly should subscribe for notification messages. There is a notification to
     * cease SD card operations so that the active SD card can be switched. If those tasks receive
     * this notifications they should cease operations and only resume when the active SD card has
     * switched.
     * @param queueId
     */
    void subscribeForSdCardNotifications(MessageQueueId_t queueId);
#endif

    /* Useful functions for development */
    static ReturnValue_t printRepository(const char* repository);
    static ReturnValue_t printSdCard();

    /* This function will dump the current SD card format in ASCII format */
    static ReturnValue_t dumpSdCard();
private:
    /**
     * The MessageQueue used to receive commands, data and to send replies.
     */
    MessageQueueIF* commandQueue;
    ActionHelper actionHelper;
    Countdown countdown;
    SDCHStateMachine stateMachine;

    /* Receiver for completed Actions */
    MessageQueueId_t actionSender = MessageQueueIF::NO_QUEUE;
    ActionId_t currentAction = -1;
    bool sdCardChangeOngoing = false;

    MessageQueueId_t fileSystemSender = MessageQueueIF::NO_QUEUE;

#ifdef ISIS_OBC_G20
    std::vector<MessageQueueId_t> sdCardNotificationRecipients;
#endif

    PeriodicTaskIF* executingTask = nullptr;
    dur_millis_t periodMs = 0;

    StorageManagerIF *ipcStore;

    /* Core functions called in performOperation */
    ReturnValue_t handleNextMessage(CommandMessage* message);

    /* Right now, only supports one manual file upload or read at a time. */
    static constexpr uint16_t UNSET_SEQUENCE = -1;
    uint16_t lastPacketWriteNumber = UNSET_SEQUENCE;
    uint16_t lastPacketReadNumber = UNSET_SEQUENCE;
    /* This will cache the offset of the current file. The offset can also
    be calculated manually by multiplying the read sequence number with MAX_READ_LENGTH. */
    size_t currentReadPos = 0;

    ReturnValue_t handleMessage(CommandMessage* message);
    ReturnValue_t handleFileMessage(CommandMessage* message);

    /** HasFilesystemIF overrides */
    ReturnValue_t createFile(const char* repositoryPath, const char* filename, const uint8_t* data,
            size_t size, void* args = nullptr) override;
    ReturnValue_t appendToFile(const char* repositoryPath, const char* filename,
            const uint8_t* data, size_t size, uint16_t packetNumber, void* args = nullptr) override;
    ReturnValue_t handleSequenceNumberWrite(uint16_t sequenceNumber, uint16_t* packetSeqIfMissing);

    ReturnValue_t deleteFile(const char* repositoryPath, const char* filename,
            void* args = nullptr) override;

    ReturnValue_t createDirectory(const char* repositoryPath, const char* dirname);
    ReturnValue_t deleteDirectory(const char* repositoryPath, const char* dirname);
    ReturnValue_t changeDirectory(const char* repositoryPath);

    ReturnValue_t createDirectory(const char* repositoryPath, void* args = nullptr) override;
    ReturnValue_t removeDirectory(const char* repositoryPath,
            bool deleteRecurively = false, void* args = nullptr) override;

    ReturnValue_t handleCreateFileCommand(CommandMessage* message);
    ReturnValue_t handleDeleteFileCommand(CommandMessage* message);

    ReturnValue_t handleReportAttributesCommand(CommandMessage* message);
    ReturnValue_t handleLockFileCommand(CommandMessage* message, bool lock);

    ReturnValue_t handleCreateDirectoryCommand(CommandMessage* message);
    ReturnValue_t handleDeleteDirectoryCommand(CommandMessage* message);
    ReturnValue_t handleCopyCommand(CommandMessage* message);

    ReturnValue_t handleAppendCommand(CommandMessage* message);
    ReturnValue_t handleFinishAppendCommand(CommandMessage* message);

    ReturnValue_t handleReadCommand(CommandMessage* message);
    ReturnValue_t handleSequenceNumberRead(uint16_t sequenceNumber);
    ReturnValue_t openFileForReading(const char* repositoryPath, const char* filename,
            F_FILE** file, size_t readPosition, size_t* fileSize, size_t* sizeToRead);
    ReturnValue_t handleReadReplies(ReadCommand& command);

    void sendCompletionReply(bool success = true,
            ReturnValue_t errorCode = HasReturnvaluesIF::RETURN_OK, uint32_t errorParam = 0);
    /** Specifying NO_QUEUE as queueId will cause a reply to the last sender */
    void sendCompletionMessage(bool success, MessageQueueId_t queueId,
            ReturnValue_t errorCode = HasReturnvaluesIF::RETURN_OK, uint32_t errorParam = 0);

    ReturnValue_t generateFinishAppendReply(RepositoryPath* repoPath, FileName* fileName,
            size_t filesize, bool locked);

    ReturnValue_t getStoreData(store_address_t& storeId, ConstStorageAccessor& accessor,
            const uint8_t** ptr, size_t* size);

    ReturnValue_t handleSdCardAccessResult(SDCardAccess& sdCardAccess);
    void driveStateMachine();

    /* Static helper function to print out the SD card */
    static ReturnValue_t printFilesystemHelper(uint8_t recursionDepth);
};

#endif /* SAM9G20_MEMORY_SDCARDHANDLER_H_ */
