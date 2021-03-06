#ifndef SAM9G20_MEMORY_SDCARDHANDLERPACKETS_H_
#define SAM9G20_MEMORY_SDCARDHANDLERPACKETS_H_

#include "sdcardDefinitions.h"

#include <fsfw/serialize/SerialLinkedListAdapter.h>
#include <fsfw/serialize/SerialFixedArrayListAdapter.h>
#include <fsfw/serialize/EndianConverter.h>
#include <fsfw/serviceinterface/ServiceInterface.h>
#include <bsp_sam9g20/common/SDCardApi.h>


/**
 * Common helper function to deserialize repository and filename
 * @param buffer
 * @param size
 * @param path
 * @param filename
 * @return
 */
static ReturnValue_t deSerializeRepositoryAndFilename(const uint8_t **buffer,
        size_t* size, RepositoryPath& path, FileName& filename);
/**
 * Common helper function to deserialize two repositories.
 * @param buffer
 * @param size
 * @param repositoryPath
 * @param dirname
 * @return
 */
static ReturnValue_t deSerializeRepositories(const uint8_t **buffer,
        size_t* size, RepositoryPath& repositoryPath,
        RepositoryPath& dirname);

static ReturnValue_t serializeRepositoryAndFilename(uint8_t **buffer,
        size_t* size, size_t maxSize, const RepositoryPath& repositoryPath,
        const FileName& filename);

class ActivePreferedVolumeReport: public SerialLinkedListAdapter<SerializeIF> {
public:
    ActivePreferedVolumeReport(VolumeId volumeId): volumeId(volumeId) {
        setStart(&(this->volumeId));
    }
private:
    SerializeElement<uint8_t> volumeId;
};


/**
 * @brief 	Generic class for all packets containg a repository and a file
 * 			name
 * @details
 * Content:
 *  1. The repository path as string
 * 	2. The name of the file
 */
class GenericFilePacket: public SerializeIF {
public:
    ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
            Endianness streamEndianness) override {
        return deSerializeRepositoryAndFilename(buffer, size,
                repositoryPath, filename);
    }

    size_t getSerializedSize() const override {
        return 0;
    }

    ReturnValue_t serialize(uint8_t **buffer, size_t *size,
            size_t maxSize, Endianness streamEndianness) const override {
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    const char* getRepositoryPathRaw() {
        return repositoryPath.c_str();
    }

    const char* getFilenameRaw() {
        return filename.c_str();
    }

    RepositoryPath* getRepoPath() {
        return &repositoryPath;
    }

    FileName* getFilename() {
        return &filename;
    }

protected:
    RepositoryPath repositoryPath;
    FileName filename;
};

/**
 * @brief 	Generic class for all packets containg a repository and directory
 * 			name
 * @details
 * Content:
 *  1. The repository path as string
 * 	2. The name of the file
 */
class GenericDirectoryPacket: public SerializeIF {
public:
    ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
            Endianness streamEndianness) override {
        return deSerializeRepositories(buffer ,size, repositoryPath, dirname);
    }

    size_t getSerializedSize() const override {
        return 0;
    }

    ReturnValue_t serialize(uint8_t **buffer, size_t *size,
            size_t maxSize, Endianness streamEndianness) const override {
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    const char* getRepositoryPath() {
        return repositoryPath.c_str();
    }

    const char* getDirname() {
        return dirname.c_str();
    }

protected:
    RepositoryPath repositoryPath;
    RepositoryPath dirname;
};

class GenericSourceTargetCommand: public SerializeIF {
public:
    ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
            Endianness streamEndianness) override {
        ReturnValue_t result =  deSerializeRepositoryAndFilename(buffer, size,
                sourcePath, sourceName);
        if(result != HasReturnvaluesIF::RETURN_OK) {
            return result;
        }
        return deSerializeRepositoryAndFilename(buffer, size, targetPath, targetName);
    }

    size_t getSerializedSize() const override {
        return 0;
    }

    ReturnValue_t serialize(uint8_t **buffer, size_t *size,
            size_t maxSize, Endianness streamEndianness) const override {
        if(sourceName.size() == 0 or sourcePath.size() == 0 or
                targetPath.size() == 0 or targetName.size() == 0) {
            return HasReturnvaluesIF::RETURN_FAILED;
        }

        ReturnValue_t result = serializeRepositoryAndFilename(buffer, size, maxSize, sourcePath,
                sourceName);
        if(result != HasReturnvaluesIF::RETURN_OK) {
            return result;
        }
        return serializeRepositoryAndFilename(buffer, size, maxSize, targetPath, targetName);
    }

    void setForSerialization(RepositoryPath& sourcePath, FileName& sourceName,
            RepositoryPath& targetPath, FileName& targetName) {
        this->sourcePath = sourcePath;
        this->sourceName = sourceName;
        this->targetPath = targetPath;
        this->targetName = targetName;
    }

    const char* getSourceRepositoryPathRaw() {
        return sourcePath.c_str();
    }
    const char* getSourceFilenameRaw() {
        return sourceName.c_str();
    }
    RepositoryPath* getSourceRepoPath() {
        return &sourcePath;
    }
    FileName* getSourceFilename() {
        return &sourceName;
    }

    const char* getTargetRepositoryPathRaw() {
        return targetPath.c_str();
    }
    const char* getTargetFilenameRaw() {
        return targetName.c_str();
    }
    RepositoryPath* getTargetRepoPath() {
        return &targetPath;
    }
    FileName* getTargetFilename() {
        return &targetName;
    }
protected:
    RepositoryPath sourcePath;
    FileName sourceName;
    RepositoryPath targetPath;
    FileName targetName;
};

class DeleteFileCommand: public GenericFilePacket {};
class FileAttributesCommand: public GenericFilePacket {};
class LockFileCommand: public GenericFilePacket {};
class CopyFileCommand: public GenericSourceTargetCommand {};
class MoveFileCommand: public GenericSourceTargetCommand {};

class CreateDirectoryCommand: public GenericDirectoryPacket {};
class DeleteDirectoryCommand: public GenericDirectoryPacket {};


/**
 * @brief   This class extracts the data buffer containing a file system
 * 			write command
 *
 * @details The data buffer of a write command contains:
 * 			1. The repository path
 * 			2. The filename
 * 			3. The packet number: For large files the data to write is split
 * 			   in multiple packets. The packet number counts the number of
 * 			   packets.
 * 			4. The data to write to the file
 */
class WriteCommand: public SerializeIF {
public:
    enum WriteType {
        NEW_FILE,
        APPEND_TO_FILE
    };

    /**
     * @brief This is the contructor for deserialization
     *
     * @param writeType
     */
    WriteCommand(WriteType writeType): writeType(writeType) {}

    /**
     * @brief This is the contructor for serialization
     *
     * @param repositoryPath
     * @param fileName
     * @param fileData
     * @param fileDataSize
     * @param writeType
     * @param packetSequenceNumber
     */
    WriteCommand(RepositoryPath& repositoryPath, FileName& fileName, const uint8_t* fileData,
            const size_t fileDataSize, WriteType writeType, uint16_t packetSequenceNumber = 0):
                repositoryPath(repositoryPath), fileName(fileName), fileData(fileData),
                writeType(writeType), fileDataSize(fileDataSize) {}

    ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
            Endianness streamEndianness) override {
        ReturnValue_t result = deSerializeRepositoryAndFilename(buffer, size,
                repositoryPath, fileName);
        if(result != HasReturnvaluesIF::RETURN_OK) {
            return result;
        }

        /* Deserialize packet number */
        if(writeType == WriteType::APPEND_TO_FILE) {
            result = SerializeAdapter::deSerialize(&packetSequenceNumber,
                    buffer, size, streamEndianness);
            if(result != HasReturnvaluesIF::RETURN_OK) {
                return result;
            }
        }


        /* Just keep internal pointer of rest of data, no copying */
        fileDataSize = *size;
        fileData = *buffer;
        return HasReturnvaluesIF::RETURN_OK;
    }

    size_t getSerializedSize() const override {
        return 0;
    }

    ReturnValue_t serialize(uint8_t **buffer, size_t *size,
            size_t maxSize, Endianness streamEndianness) const override {

        ReturnValue_t result = serializeRepositoryAndFilename(buffer, size, maxSize,
                repositoryPath, fileName);

        if(result != HasReturnvaluesIF::RETURN_OK) {
            return result;
        }

        /* Serialize packet number */
        if(writeType == WriteType::APPEND_TO_FILE) {
            result = SerializeAdapter::serialize(&packetSequenceNumber,
                    buffer, size, maxSize, streamEndianness);

            if(result != HasReturnvaluesIF::RETURN_OK) {
                return result;
            }
        }
        size_t newSize = fileDataSize + *size;
        if (newSize > maxSize or newSize < *size) {
            return SerializeIF::BUFFER_TOO_SHORT;
        }

        std::memcpy(*buffer, fileData, fileDataSize);
        *size = newSize;
        *buffer += fileDataSize;

        return HasReturnvaluesIF::RETURN_OK;
    }

    const char* getRepositoryPath(){
        return repositoryPath.c_str();
    }

    const char* getFilename(){
        return fileName.c_str();
    }

    const uint8_t* getFileData(){
        return fileData;
    }

    size_t getFileSize(){
        return fileDataSize;
    }

    uint16_t getPacketNumber(){
        return packetSequenceNumber;
    }

private:
    RepositoryPath repositoryPath;
    FileName fileName;
    uint16_t packetSequenceNumber = 0;
    const uint8_t* fileData = nullptr;

    WriteType writeType; //! [EXPORT] : [IGNORE]
    size_t fileDataSize = 0; //! [EXPORT] : [IGNORE]
};


/**
 * @brief	This helps encapsulates a finish-append command.
 */
class FinishAppendCommand: public GenericFilePacket {
public:
    ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
            Endianness streamEndianness) override {
        ReturnValue_t result = GenericFilePacket::deSerialize(buffer ,size,
                streamEndianness);
        if(result != HasReturnvaluesIF::RETURN_OK) {
            return result;
        }
        return SerializeAdapter::deSerialize(&lockFile, buffer ,size,
                streamEndianness);
    }

    bool getLockFile() const {
        return lockFile;
    }

private:
    bool lockFile = false;
};

class FinishAppendReply: public SerialLinkedListAdapter<SerializeIF> {
public:
    FinishAppendReply(const RepositoryPath* repoPath, const FileName* fileName,
            uint16_t lastValidSequenceNumber, size_t fileSize, bool fileLocked):
                repoPath(repoPath), fileName(fileName),
                lastValidSequenceNumber(lastValidSequenceNumber), fileSize(fileSize),
                fileLocked(fileLocked) {
        setStart(&this->lastValidSequenceNumber);
        this->lastValidSequenceNumber.setNext(&this->fileSize);
        this->fileSize.setNext(&this->fileLocked);
        this->fileLocked.setEnd();
    }

    ReturnValue_t serialize(uint8_t **buffer, size_t *size,
            size_t maxSize, Endianness streamEndianness) const override {
        ReturnValue_t result = serializeRepositoryAndFilename(buffer, size,
                maxSize, const_cast<RepositoryPath&>(*repoPath),
                const_cast<FileName&>(*fileName));
        if(result != HasReturnvaluesIF::RETURN_OK) {
            return result;
        }

        return SerialLinkedListAdapter::serialize(buffer, size, maxSize,
                streamEndianness);
    }

    size_t getSerializedSize() const override {
        return repoPath->size() + fileName->size() + 2 +
                SerialLinkedListAdapter::getSerializedSize();
    }

    ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
            Endianness streamEndianness) override {
        return HasReturnvaluesIF::RETURN_FAILED;
    }


    const RepositoryPath* repoPath;
    const FileName* fileName;
private:

    SerializeElement<uint16_t> lastValidSequenceNumber;
    SerializeElement<size_t> fileSize;
    SerializeElement<uint8_t> fileLocked;

};

/**
 * @brief This Class extracts the repository path and the filename from the
 *        data buffer of a read command file system message
 * @details
 */
class ReadCommand: public GenericFilePacket {
public:
    ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
            Endianness streamEndianness) override {
        ReturnValue_t result = GenericFilePacket::deSerialize(buffer ,size,
                streamEndianness);
        if(result != HasReturnvaluesIF::RETURN_OK) {
            return result;
        }
        return SerializeAdapter::deSerialize(&sequenceNumber, buffer ,size,
                streamEndianness);
    }

    uint16_t getSequenceNumber() {
        return sequenceNumber;
    }
private:
    uint16_t sequenceNumber = 0;
};


/**
 * @brief   This class serves as a helper to put the parameters of a reply to
 *          the file system read command into one common buffer.
 */
class ReadReply: public SerializeIF {
public:

    ReadReply(RepositoryPath* repoPath, FileName* fileName,
            F_FILE** fileHandle, size_t sizeToRead) :
                repoPath(repoPath), fileName(fileName),
                sizeToRead(sizeToRead), fileHandle(*fileHandle) {
    }

    ReturnValue_t serialize(uint8_t **buffer, size_t *size,
            size_t maxSize, Endianness streamEndianness) const override {
        ReturnValue_t result = serializeRepositoryAndFilename(buffer , size,
                maxSize, *repoPath, *fileName);
        if(result != HasReturnvaluesIF::RETURN_OK) {
            return result;
        }

        if(*size + sizeToRead > maxSize) {
            // Should not happen!
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "SDCardHandlerPackets::ReadReply: Max Size specified "
                    << "is not large enough" << std::endl;
#else
            sif::printError("SDCardHandlerPackets::ReadReply: Max Size specified "
                    "is not large enough\n");
#endif
            return SerializeIF::BUFFER_TOO_SHORT;
        }

        size_t sizeRead = static_cast<size_t>(f_read(*buffer, sizeof(uint8_t),
                sizeToRead, fileHandle));
        if(sizeRead != sizeToRead) {
            /* Should not happen! */
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "SDCardHandlerPackets::ReadReply: Did not read all bytes." << std::endl;
#else
            sif::printError("SDCardHandlerPackets::ReadReply: Did not read all bytes.\n");
#endif
            return HasReturnvaluesIF::RETURN_FAILED;
        }

        *buffer += sizeRead;
        *size += sizeRead;
        return HasReturnvaluesIF::RETURN_OK;
    }

    ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
            Endianness streamEndianness) override {
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    size_t getSerializedSize() const override {
        return repoPath->size() + fileName->size() + 2 + sizeToRead;
    }


private:

    RepositoryPath* repoPath;
    FileName* fileName;
    uint8_t* fileData = nullptr;

    size_t sizeToRead;
    F_FILE* fileHandle;
};

class FileAttributesReply: public GenericFilePacket {
public:
    FileAttributesReply(RepositoryPath* repoPath, FileName* fileName,
            size_t fileSize, bool locked): repoPath(repoPath),
            fileName(fileName), fileSize(fileSize), locked(locked) {}

    ReturnValue_t serialize(uint8_t **buffer, size_t *size,
            size_t maxSize, Endianness streamEndianness) const override {
        ReturnValue_t result = serializeRepositoryAndFilename(buffer, size,
                maxSize, *repoPath, *fileName);
        if(result != HasReturnvaluesIF::RETURN_OK) {
            return result;
        }
        result = SerializeAdapter::serialize(&fileSize, buffer ,size ,
                maxSize, streamEndianness);
        if(result != HasReturnvaluesIF::RETURN_OK) {
            return result;
        }
        return SerializeAdapter::serialize(&locked, buffer ,size ,
                maxSize, streamEndianness);
    }

    size_t getSerializedSize() const override {
        return repoPath->size() + fileName->size() + 2 + sizeof(fileSize) +
                sizeof(locked);
    }
private:
    RepositoryPath* repoPath;
    FileName* fileName;
    /* In addition to the repository path and the directory name,
    this packet contains the file size and whether the file is locked. */
    uint32_t fileSize;
    bool locked;

};


ReturnValue_t deSerializeRepositoryAndFilename(const uint8_t **buffer,
        size_t* size, RepositoryPath& repositoryPath, FileName& filename) {
    if(*buffer == nullptr) {
        /* This should not happen! */
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    /* Deserialize repository first. */
    size_t repositoryLength = std::strlen(
            reinterpret_cast<const char*>(*buffer));
    if(repositoryLength > MAX_REPOSITORY_PATH_LENGTH) {
        // Packet too short or repository length to large.
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "WriteCommand: Repository path longer than "
                << MAX_REPOSITORY_PATH_LENGTH << " or no '\0' terminator" << std::endl;
#else
        sif::printWarning("WriteCommand: Repository path longer than %d or no '\0' terminator\n",
                MAX_REPOSITORY_PATH_LENGTH);
#endif
    }
    if(*size < repositoryLength) {
        return SerializeIF::STREAM_TOO_SHORT;
    }
    repositoryPath.assign(reinterpret_cast<const char*>(*buffer));
    /* +1 because repositoryPath.size() is the size of the string
    without the string terminator */
    *buffer += repositoryPath.size() + 1;
    *size -= repositoryPath.size() + 1;

    if(*buffer == nullptr) {
        // This should not happen!
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    /* Deserialize filename next */
    size_t filenameLength = std::strlen(
            reinterpret_cast<const char*>(*buffer));
    if(filenameLength > MAX_FILENAME_LENGTH) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "WriteCommand: Filename longer than "
                << MAX_FILENAME_LENGTH << " or no '\0 terminator"
                << "detected!" << std::endl;
#else
        sif::printWarning("WriteCommand: Filename longer than %d or no '\0' terminator\n",
                MAX_FILENAME_LENGTH);
#endif
        return HasReturnvaluesIF::RETURN_OK;
    }
    if(*size < filenameLength) {
        return SerializeIF::STREAM_TOO_SHORT;
    }
    filename.assign(reinterpret_cast<const char*>(*buffer));
    /* +1 because filename.size() is the size of the string without
    the string terminator */
    *buffer += filename.size() + 1;
    *size -= filename.size() + 1;
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t deSerializeRepositories(const uint8_t **buffer,
        size_t* size, RepositoryPath& repositoryPath,
        RepositoryPath& dirname)  {
    if(*buffer == nullptr) {
        // This should not happen!
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    /* Deserialize repository first. */
    size_t repositoryLength = std::strlen(
            reinterpret_cast<const char*>(*buffer));
    if(repositoryLength > MAX_REPOSITORY_PATH_LENGTH) {
        // Packet too short or repository length to large.
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "WriteCommand: Repository path longer than "
                << MAX_REPOSITORY_PATH_LENGTH << " or no '\0 terminator" << std::endl;
#else
        sif::printWarning("WriteCommand: Repository path longer than %d or no '\0' terminator\n",
                MAX_REPOSITORY_PATH_LENGTH);
#endif
    }
    if(*size < repositoryLength) {
        return SerializeIF::STREAM_TOO_SHORT;
    }
    repositoryPath.append(reinterpret_cast<const char*>(*buffer));
    /* +1 because repositoryPath.size() is the size of the string
    without the string terminator */
    *buffer += repositoryPath.size() + 1;
    *size -= repositoryPath.size() + 1;

    if(*buffer == nullptr) {
        // This should not happen!
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    size_t allowedRemainingSize = MAX_REPOSITORY_PATH_LENGTH -
            repositoryPath.size();

    /* Deserialize target directory name */
    size_t dirnameLength = std::strlen(
            reinterpret_cast<const char*>(*buffer));
    if(dirnameLength > allowedRemainingSize) {
        // Resulting path would be too long
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "CreateDirectoryCommand::deSerialize: Directory would result in repository "
                "path length too large!" << std::endl;
#else
        sif::printWarning("CreateDirectoryCommand::deSerialize: Directory would result in "
                "repository path length too large!\n");
#endif
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    if(*size < dirnameLength) {
        return SerializeIF::STREAM_TOO_SHORT;
    }
    dirname.append(reinterpret_cast<const char*>(*buffer));
    /* +1 because repositoryPath.size() is the size of the string
    without the string terminator */
    *buffer += dirname.size() + 1;
    *size -= dirname.size() + 1;
    return HasReturnvaluesIF::RETURN_OK;

}

ReturnValue_t serializeRepositoryAndFilename(uint8_t **buffer,
        size_t* size, size_t maxSize, const RepositoryPath& repositoryPath,
        const FileName& filename) {
    /* You need to cast away the const for the ETL librairy */
    auto nonConstPath = const_cast<RepositoryPath&>(repositoryPath);
    auto nonConstFilename = const_cast<FileName&>(filename);
    // Check remaining size is large enough and check integer
    // overflow of *size
    size_t nextSize = repositoryPath.size();
    size_t newSize =  nextSize + 1 + *size;
    if ((newSize <= maxSize) and (newSize > *size)) {
        nonConstPath.copy(reinterpret_cast<char*>(*buffer), nextSize);
        *size += nextSize + 1;
        *buffer += nextSize;
        **buffer = '\0';
        *buffer += 1;
    }
    else {
        return SerializeIF::BUFFER_TOO_SHORT;
    }

    nextSize = filename.size();
    newSize =  nextSize + *size;
    if ((newSize <= maxSize) and (newSize > *size)) {
        nonConstFilename.copy(reinterpret_cast<char*>(*buffer), nextSize);
        *size += nextSize + 1;
        *buffer += nextSize;
        **buffer = '\0';
        *buffer += 1;
    }
    else {
        return SerializeIF::BUFFER_TOO_SHORT;
    }
    return HasReturnvaluesIF::RETURN_OK;
}


#endif /* SAM9G20_MEMORY_MEMORY_SDCARDHANDLERPACKETS_H_ */
