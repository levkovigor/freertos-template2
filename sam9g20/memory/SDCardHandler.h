#ifndef SAM9G20_MEMORY_SDCARDHANDLER_H_
#define SAM9G20_MEMORY_SDCARDHANDLER_H_

#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/memory/AcceptsMemoryMessagesIF.h>
#include <fsfw/ipc/MessageQueueIF.h>
#include <fsfw/memory/HasFileSystemIF.h>
#include <sam9g20/memory/SDCardApi.h>

extern "C"{
    #include <hcc/api_fat.h>
}

#include <config/events/subsystemIdRanges.h>


/**
 * Additional abstraction layer to encapsulate access to SD cards
 * using the iOBC HCC FAT API.
 */
class SDCardHandler : public SystemObject,
        public ExecutableObjectIF,
        public HasFileSystemIF {
    friend class SDCardAccess;
public:

    static constexpr uint8_t MAX_FILE_MESSAGES_HANDLED_PER_CYCLE = 5;

    static constexpr uint8_t INTERFACE_ID = CLASS_ID::SD_CARD_HANDLER;

    static const uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::SD_CARD_HANDLER;

    static constexpr Event SD_CARD_SWITCHED = MAKE_EVENT(0x00, SEVERITY::MEDIUM); //!< It was not possible to open the preferred SD card so the other was used. P1: Active volume
    static constexpr Event SD_CARD_ACCESS_FAILED = MAKE_EVENT(0x01, SEVERITY::HIGH); //!< Opening failed for both SD cards.

    SDCardHandler(object_id_t objectId_);
    virtual ~SDCardHandler();

    MessageQueueId_t getCommandQueue() const;

    ReturnValue_t performOperation(uint8_t operationCode = 0);

    // Useful functions for development
    static ReturnValue_t printRepository(const char* repository);
    static ReturnValue_t printSdCard();
private:

    ReturnValue_t handleMessage(CommandMessage* message);

    ReturnValue_t handleDeleteFileCommand(CommandMessage* message);
    ReturnValue_t handleCreateDirectoryCommand(CommandMessage* message);
    ReturnValue_t handleDeleteDirectoryCommand(CommandMessage* message);
    ReturnValue_t handleWriteCommand(CommandMessage* message);
    ReturnValue_t handleReadCommand(CommandMessage* message);
    ReturnValue_t createFile(const char* repositoryPath, const char* filename,
            const uint8_t* data, size_t size, size_t* bytesWritten) override;
    ReturnValue_t deleteFile(const char* repositoryPath, const char* filename);
    ReturnValue_t createDirectory(const char* repositoryPath,
            const char* dirname);
    ReturnValue_t deleteDirectory(const char* repositoryPath,
            const char* dirname);
    ReturnValue_t writeToFile(const char* repositoryPath, const char* filename,
            const uint8_t* data, size_t size, uint16_t packetNumber) override;
    ReturnValue_t read(const char* repositoryPath, const char* filename,
            uint8_t* tmData, size_t* tmDataLen);
    void sendCompletionReply(bool success = true,
            ReturnValue_t errorCode = HasReturnvaluesIF::RETURN_OK);
    ReturnValue_t sendDataReply(MessageQueueId_t receivedFromQueueId,
            uint8_t* tmData, size_t tmDataLen);

    /**
     * @brief   This function can be used to switch to a directory provided
     *          with the repositoryPath
     * @param repositoryPath
     * Pointer to a string holding the repositoryPath to the directory.
     * The repositoryPath must be absolute.
     * @return
     * -@c RETURN_OK if command was executed successfully.
     * -@c Filesystem errorcode otherwise
     */
    ReturnValue_t changeDirectory(const char* repositoryPath);

    /**
     * The MessageQueue used to receive commands, data and to send replies.
     */
    MessageQueueIF* commandQueue;

    uint32_t queueDepth = 20;

    StorageManagerIF *IPCStore;

    F_FILE *file = nullptr;

    /* For now  max size of reply is set to 300.
     * For larger sizes the software needs to be adapted. */
    uint32_t readReplyMaxLen = 300;

    bool fileSystemWasUsedOnce = false;
    bool fileSystemOpen = false;

    // TODO: make this configurable parameter.
    VolumeId preferredVolume = SD_CARD_0;
    VolumeId activeVolume = SD_CARD_0;

    VolumeId determineVolumeToOpen();
    ReturnValue_t handleAccessResult(ReturnValue_t accessResult);
    ReturnValue_t handleMultipleMessages(CommandMessage* message);

    static ReturnValue_t printHelper(F_FIND* result, uint8_t recursionDepth);

};

#endif /* SAM9G20_MEMORY_SDCARDHANDLER_H_ */
