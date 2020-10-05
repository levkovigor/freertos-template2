#ifndef SAM9G20_MEMORY_MEMORY_SDCARDHANDLERPACKETS_H_
#define SAM9G20_MEMORY_MEMORY_SDCARDHANDLERPACKETS_H_

#include <fsfw/serialize/SerialLinkedListAdapter.h>
#include <fsfw/serialize/SerialFixedArrayListAdapter.h>
#include <fsfw/serialize/EndianConverter.h>

extern "C"{
#include <privlib/hcc/include/config/config_fat.h>
}

#include <etl/string.h>
#include <config/OBSWConfig.h>

using RepositoryPath = etl::string<MAX_REPOSITORY_PATH_LENGTH>;
using FileName = etl::string<MAX_FILENAME_LENGTH>;

/**
 * Common helper function to deserialize repository and filename
 * @param buffer
 * @param size
 * @param path
 * @param filename
 * @return
 */
ReturnValue_t deSerializeRepositoryAndFilename(const uint8_t **buffer,
        size_t* size, RepositoryPath& path, FileName& filename);

/**
 * @brief   This class helps to handle a delete-file command.
 *
 * @details A delete-file command holds:
 * 	        1. The repository path as string
 * 	        2. The name of the file to delete as string
 * @author  Jakob Meier
 */
class DeleteFileCommand: public SerializeIF {
public:
	DeleteFileCommand() {}

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

	const char* getRepositoryPath() {
		return repositoryPath.c_str();
	}

	const char* getFilename() {
		return filename.c_str();
	}

private:

	RepositoryPath repositoryPath;
	FileName filename;
};


/**
 * @brief This class helps to handle a create-directory command.
 *
 * @details A create-directory holds:
 * 	        1. The repository path as string
 * 	        2. The directory to create as string
 */
class CreateDirectoryCommand: public SerializeIF {
public:
	CreateDirectoryCommand() {}

    ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
            Endianness streamEndianness) override {
        if(*buffer == nullptr) {
            // This should not happen!
            return HasReturnvaluesIF::RETURN_FAILED;
        }

        /* Deserialize repository first. */
        size_t repositoryLength = std::strlen(
                reinterpret_cast<const char*>(*buffer));
        if(repositoryLength > MAX_REPOSITORY_PATH_LENGTH) {
            // Packet too short or repository length to large.
            sif::warning << "WriteCommand: Repository path longer than "
                    << MAX_REPOSITORY_PATH_LENGTH << " or no '\0 terminator"
                    << std::endl;
        }
        if(*size < repositoryLength) {
            return SerializeIF::STREAM_TOO_SHORT;
        }
        repositoryPath.append(reinterpret_cast<const char*>(*buffer));
        /* +1 because repositoryPath.size() is the size of the string
        without the string terminator */
        *buffer += repositoryPath.size() + 1;
        *size -= repositoryPath.size() + 1;

        size_t allowedRemainingSize = MAX_REPOSITORY_PATH_LENGTH -
                repositoryPath.size();

        /* Deserialize target directory name */
        size_t dirnameLength = std::strlen(
                reinterpret_cast<const char*>(*buffer));
        if(dirnameLength > allowedRemainingSize) {
            // Resulting path would be too long
            sif::warning << "CreateDirectoryCommand::deSerialize: Directory "
                    << " would result in repository path length too large!"
                    << std::endl;
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

private:
	RepositoryPath repositoryPath;
	RepositoryPath dirname;
};


/**
 * @brief This class helps to handle a delete-directory command.
 *
 * @details A delete-directory holds:
 * 	        1. The repository path as string
 * 	        2. The directory to delete as string
 */
class DeleteDirectoryCommand {
public:
	DeleteDirectoryCommand() {}

	ReturnValue_t deSerialize(const uint8_t** dataBuffer, size_t size){
		repositoryPath = std::string(reinterpret_cast<const char*>(*dataBuffer));
		*dataBuffer = *dataBuffer + repositoryPath.size() + 1;
		dirname = std::string(reinterpret_cast<const char*>(*dataBuffer));
		if(repositoryPath.size() + dirname.size() + 2 != size){
			return HasReturnvaluesIF::RETURN_FAILED;
		}
		return HasReturnvaluesIF::RETURN_OK;
	}

	const char* getRepositoryPath() {
		return repositoryPath.c_str();
	}

	const char* getDirname() {
		return dirname.c_str();
	}

private:

	DeleteDirectoryCommand(const DeleteDirectoryCommand &command);

	std::string repositoryPath;
	std::string dirname;
};

/**
 * @brief   This class extracts the data buffer containing a file system write command
 *
 * @details The data buffer of a write command contains:
 * 			1. The repository path
 * 			2. The filename
 * 			3. The packet number: For large files the data to write is split in multiple packets.
 * 			   The packet number counts the number of packets.
 * 			4. The data to write to the file
 */
class WriteCommand: public SerializeIF {
public:
    enum WriteType {
        NEW_FILE,
        APPEND_TO_FILE
    };

	WriteCommand(WriteType writeType): writeType(writeType){}

	ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
            Endianness streamEndianness) override {
	    ReturnValue_t result = deSerializeRepositoryAndFilename(buffer, size,
	            repositoryPath, filename);
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
		filesize = *size;
		fileData = *buffer;
		return HasReturnvaluesIF::RETURN_OK;
	}

	size_t getSerializedSize() const override {
	    return 0;
	}

	ReturnValue_t serialize(uint8_t **buffer, size_t *size,
            size_t maxSize, Endianness streamEndianness) const override {
	    return HasReturnvaluesIF::RETURN_FAILED;
	}

	const char* getRepositoryPath(){
		return repositoryPath.c_str();
	}

	const char* getFilename(){
		return filename.c_str();
	}

	const uint8_t* getFileData(){
		return fileData;
	}

	uint32_t getFileSize(){
		return filesize;
	}

	uint16_t getPacketNumber(){
		return packetSequenceNumber;
	}

private:

	WriteType writeType;

	RepositoryPath repositoryPath;
	FileName filename;
	uint16_t packetSequenceNumber = 0;
	size_t filesize = 0; //! [EXPORT] : [IGNORE]
	const uint8_t* fileData = nullptr;
};

/**
 * @brief This Class extracts the repository path and the filename from the
 *        data buffer of a read command file system message
 */
class ReadCommand: public SerializeIF {
public:

	ReadCommand() {}

	ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
	            Endianness streamEndianness) override {
		return deSerializeRepositoryAndFilename(buffer, size, repositoryPath,
		        filename);
	}

    size_t getSerializedSize() const override {
        return 0;
    }

    ReturnValue_t serialize(uint8_t **buffer, size_t *size,
            size_t maxSize, Endianness streamEndianness) const override {
        return HasReturnvaluesIF::RETURN_FAILED;
    }

	const char * getRepositoryPath() {
		return repositoryPath.c_str();
	}

	const char* getFilename() {
		return filename.c_str();
	}

private:

	RepositoryPath repositoryPath;
	FileName filename;
};

/**
 * @brief   This class serves as a helper to put the parameters of a reply to
 *          the file system read command into one common buffer.
 */
class ReadReply {
public:

	ReadReply(const char* repositoryPath_, const char* filename_,
	        uint8_t* data_, uint16_t filesize_) :
			repositoryPath(repositoryPath_), filename(filename_),
			filesize(filesize_) {
		std::memcpy(data, data_, filesize_);
	}

	ReturnValue_t serialize(uint8_t* tmData, size_t* tmDataLen) {
		uint8_t* tmp = tmData;
		repositoryPath.copy(reinterpret_cast<char*>(tmp), repositoryPath.size());
		tmp = tmp + repositoryPath.size();
		/* Adding string terminator */
		*tmp = 0;
		tmp = tmp + 1;
		filename.copy(reinterpret_cast<char*>(tmp), filename.size());
		tmp = tmp + filename.size();
		/* Adding string terminator */
		*tmp = 0;
		tmp = tmp + 1;
		std::memcpy(tmp, data, filesize);
		/* +2 for the two string terminators */
		*tmDataLen = repositoryPath.size() + filename.size() + filesize + 2;
		return HasReturnvaluesIF::RETURN_OK;
	}

private:

	const static uint32_t maxFileSize = 300;
	std::string repositoryPath;
	std::string filename;
	uint16_t filesize;
	uint8_t data[maxFileSize];
};


ReturnValue_t deSerializeRepositoryAndFilename(const uint8_t **buffer,
        size_t* size, RepositoryPath& repositoryPath, FileName& filename) {
    if(*buffer == nullptr) {
        // This should not happen!
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    /* Deserialize repository first. */
    size_t repositoryLength = std::strlen(
            reinterpret_cast<const char*>(*buffer));
    if(repositoryLength > MAX_REPOSITORY_PATH_LENGTH) {
        // Packet too short or repository length to large.
        sif::warning << "WriteCommand: Repository path longer than "
                << MAX_REPOSITORY_PATH_LENGTH << " or no '\0 terminator"
                << std::endl;
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

    /* Deserialize filename next */
    size_t filenameLength = std::strlen(
            reinterpret_cast<const char*>(*buffer));
    if(filenameLength > MAX_FILENAME_LENGTH) {
        sif::warning << "WriteCommand: Repository path longer than "
                << MAX_FILENAME_LENGTH << " or no '\0 terminator"
                << "detected!" << std::endl;
        return HasReturnvaluesIF::RETURN_OK;
    }
    if(*size < filenameLength) {
        return SerializeIF::STREAM_TOO_SHORT;
    }
    filename.append(reinterpret_cast<const char*>(*buffer));
    /* +1 because filename.size() is the size of the string without
    the string terminator */
    *buffer += filename.size() + 1;
    *size -= filename.size() + 1;
    return HasReturnvaluesIF::RETURN_OK;
}

#endif /* SAM9G20_MEMORY_MEMORY_SDCARDHANDLERPACKETS_H_ */
