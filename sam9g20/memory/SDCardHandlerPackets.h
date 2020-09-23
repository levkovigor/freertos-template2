#ifndef SAM9G20_MEMORY_MEMORY_SDCARDHANDLERPACKETS_H_
#define SAM9G20_MEMORY_MEMORY_SDCARDHANDLERPACKETS_H_

#include <fsfw/serialize/SerialLinkedListAdapter.h>
#include <fsfw/serialize/SerialFixedArrayListAdapter.h>
#include <fsfw/serialize/EndianConverter.h>

extern "C"{
#include <privlib/hcc/include/config/config_fat.h>
}


/**
 * @brief   This class helps to handle a delete-file command.
 *
 * @details A delete-file command holds:
 * 	        1. The repository path as string
 * 	        2. The name of the file to delete as string
 * @author  Jakob Meier
 */
class DeleteFileCommand {
public:
	DeleteFileCommand() {}

	ReturnValue_t deSerialize(const uint8_t** dataBuffer, size_t size){
		repositoryPath = std::string(reinterpret_cast<const char*>(*dataBuffer));
		*dataBuffer = *dataBuffer + repositoryPath.size() + 1;
		filename = std::string(reinterpret_cast<const char*>(*dataBuffer));
		if(repositoryPath.size() + filename.size() + 2 != size){
			return HasReturnvaluesIF::RETURN_FAILED;
		}
		return HasReturnvaluesIF::RETURN_OK;
	}

	const char* getRepositoryPath() {
		return repositoryPath.c_str();
	}

	const char* getFilename() {
		return filename.c_str();
	}

private:

	DeleteFileCommand(const DeleteFileCommand &command);

	std::string repositoryPath;
	std::string filename;
};


/**
 * @brief This class helps to handle a create-directory command.
 *
 * @details A create-directory holds:
 * 	        1. The repository path as string
 * 	        2. The directory to create as string
 */
class CreateDirectoryCommand {
public:
	CreateDirectoryCommand() {}

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

	CreateDirectoryCommand(const CreateDirectoryCommand &command);

	std::string repositoryPath;
	std::string dirname;
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
class WriteCommand : public SerialLinkedListAdapter<SerializeIF>{
public:

	WriteCommand(){}

	ReturnValue_t deSerialize(const uint8_t* dataBuffer, uint32_t size){
		repositoryPath = std::string(reinterpret_cast<const char*>(dataBuffer));
		/* +1 because repositoryPath.size() is the size of the string without the string terminator */
		dataBuffer = dataBuffer + repositoryPath.size() + 1;
		filename = std::string(reinterpret_cast<const char*>(dataBuffer));
		/* +1 because filename.size() is the size of the string without the string terminator */
		dataBuffer = dataBuffer + filename.size() + 1;
		std::memcpy(&packetNumber, dataBuffer, sizeof(packetNumber));
		dataBuffer = dataBuffer + sizeof(packetNumber);
		/* -2 because of the two string terminators within the dataBuffer */
		filesize = size - repositoryPath.size() - filename.size() - sizeof(packetNumber) - 2;
		std::memcpy(fileData, dataBuffer, filesize);
		return HasReturnvaluesIF::RETURN_OK;
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
		return packetNumber;
	}

private:

	/* For now the size of the PUS packet is limited. */
	const static uint32_t maxFileSize = 300;
	std::string repositoryPath;
	std::string filename;
	uint16_t packetNumber = 0;
	uint32_t filesize = 0;
	uint8_t fileData[maxFileSize];
};

/**
 * @brief This Class extracts the repository path and the filename from the
 *        data buffer of a read command file system message
 */
class ReadCommand {
public:

	ReadCommand() {}

	ReturnValue_t deSerialize(const uint8_t** dataBuffer, size_t size){
		repositoryPath = std::string(reinterpret_cast<const char*>(*dataBuffer));
		*dataBuffer = *dataBuffer + repositoryPath.size() + 1;
		filename = std::string(reinterpret_cast<const char*>(*dataBuffer));
		if(repositoryPath.size() + filename.size() + 2 != size){
			return HasReturnvaluesIF::RETURN_FAILED;
		}
		return HasReturnvaluesIF::RETURN_OK;
	}

	const char * getRepositoryPath() {
		return repositoryPath.c_str();
	}

	const char* getFilename() {
		return filename.c_str();
	}

private:

	std::string repositoryPath;
	std::string filename;
};

/**
 * @brief This class serves as a helper to put the parameters of a reply to the file system read command
 *  	  into one common buffer.
 */
class ReadReply {
public:

	ReadReply(const char* repositoryPath_, const char* filename_, uint8_t* data_, uint16_t filesize_) :
			repositoryPath(repositoryPath_), filename(filename_), filesize(filesize_) {
		std::memcpy(data, data_, filesize_);
	}

	ReturnValue_t serialize(uint8_t* tmData, uint32_t* tmDataLen){
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
#endif /* SAM9G20_MEMORY_MEMORY_SDCARDHANDLERPACKETS_H_ */
