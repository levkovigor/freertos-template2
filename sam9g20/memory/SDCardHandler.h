#ifndef SAM9G20_MEMORY_SDCARDHANDLER_H_
#define SAM9G20_MEMORY_SDCARDHANDLER_H_

#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/memory/AcceptsMemoryMessagesIF.h>
#include <fsfw/ipc/MessageQueueIF.h>
#include <fsfw/memory/HasFileSystemIF.h>

extern "C"{
    #include <hcc/api_fat.h>
    #include <hcc/api_hcc_mem.h>
    #include <hcc/api_mdriver_atmel_mcipdc.h>
}


enum class VolumeId {
    SD_CARD_0 = 0,
    SD_CARD_1 = 1
};


/**
 * @brief   This access class can be created locally to initiate access to the
 *          file system.
 * @details
 * It will automatically close the access when being destroyed
 */
class FileSystemAccess {
public:
    FileSystemAccess(VolumeId volumeId = VolumeId::SD_CARD_0);
    ~FileSystemAccess();

    ReturnValue_t accessSuccess = HasReturnvaluesIF::RETURN_OK;
private:
    VolumeId volumeId;
};

/**
 * Additional abstraction layer to encapsulate access to SD cards
 * using the iOBC HCC FAT API.
 */
class SDCardHandler : public SystemObject,
        public ExecutableObjectIF,
        public HasFileSystemIF {
    friend class FileSystemAccess;
public:
    static constexpr uint8_t INTERFACE_ID = CLASS_ID::SD_CARD_HANDLER;
    static constexpr ReturnValue_t FOLDER_ALREADY_EXISTS = MAKE_RETURN_CODE(0x01);

    SDCardHandler(object_id_t objectId_);
    virtual ~SDCardHandler();

    MessageQueueId_t getCommandQueue() const;

    ReturnValue_t performOperation(uint8_t operationCode = 0);

private:
    static ReturnValue_t openFilesystem(VolumeId volumeId = VolumeId::SD_CARD_0);
    static ReturnValue_t closeFilesystem(VolumeId volumeId = VolumeId::SD_CARD_0);
    static ReturnValue_t selectSDCard(VolumeId volumeID);

    ReturnValue_t handleMessages();

    ReturnValue_t handleDeleteFileCommand(CommandMessage* message);
    ReturnValue_t handleCreateDirectoryCommand(CommandMessage* message);
    ReturnValue_t handleDeleteDirectoryCommand(CommandMessage* message);
    ReturnValue_t handleWriteCommand(CommandMessage* message);
    ReturnValue_t handleReadCommand(CommandMessage* message);
    ReturnValue_t createFile(const char* dirname, const char* filename,
            const uint8_t* data, size_t size) override;
    ReturnValue_t deleteFile(const char* repositoryPath, const char* filename);
    ReturnValue_t createDirectory(const char* repositoryPath,
            const char* dirname);
    ReturnValue_t deleteDirectory(const char* repositoryPath,
            const char* dirname);
    ReturnValue_t writeToFile(const char* repositoryPath, const char* filename,
            const uint8_t* data, size_t size, uint16_t packetNumber) override;
    ReturnValue_t read(const char* repositoryPath, const char* filename,
            uint8_t* tmData, uint32_t* tmDataLen);
    void sendCompletionReply(MessageQueueId_t replyToQueue,
            Command_t completionStatus);
    ReturnValue_t sendDataReply(MessageQueueId_t receivedFromQueueId,
            uint8_t* tmData, uint8_t tmDataLen);

    /**
     * @brief   This function can be used to switch to a directory provided
     *          with the repositoryPath
     * @param repositoryPath
     * Pointer to a string holding the repositoryPath to the directory.
     * The repositoryPath must be absolute.
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

    bool fileSystemInitialized = false;
};

#endif /* SAM9G20_MEMORY_SDCARDHANDLER_H_ */
