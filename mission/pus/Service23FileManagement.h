#ifndef MISSION_PUS_SERVICE23FILEMANAGEMENT_H_
#define MISSION_PUS_SERVICE23FILEMANAGEMENT_H_

#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/tmtcservices/CommandingServiceBase.h>

/**
 * @brief File Management Service
 *
 * Full Documentation: ECSS-E-ST-70-41C p.403, p.653
 *
 * The file management service provides the capability to manage on-board file
 * systems and files.
 *
 * The following standardized PUS subservices were implemented at allow
 * basic file handling:
 *   - TC[23,1]: Create a file
 *   - TC[23,2]: Delete a file
 *   - TC[23,3]: Report the attributes of a file
 *   - TM[23,4]: Response to TC[23,3]
 *   - TC[23,5]: Lock a file
 *   - TC[23,6]: Unlock a file
 *   - TC[23,7]: Find files
 *   - TM[23,8]: Response to TC[23,7]
 *   - TC[23,9]: Create a directory
 *   - TC[23,10]: Delete a directory
 *   - TC[23,11]: Rename a directory
 *   - TC[23,12]: Summary-report the content of a repository
 *   - TM[23,13]: Response of TC[23,12]
 *   - TC[23,15]: Move a file
 *
 * A set of custom subservices will be implemented for uploading files:
 *  - TC[23,130]: Append to file. The service will  implement sequence checking
 *    for consecutive append operations on the same file but only support one
 *    append operation at a time. Please note that if an append operation
 *    was stopped and has to be continued, the packet counter should
 *    start at 0 so that the last packet sequence counter is reset.
 *  - TC[23,131]: Stop or finish append operation. This service will stop or
 *    finish the append operation so that another append operation can be
 *    started. A telemetry packet containing the repository, the file path,
 *    last valid sequence number, and the current file size will be generated.
 *  - TC[23,132]: Stop append reply.
 *
 * A set of custom subservices will be implemented for downloading files:
 *  - TC[23,135]: Automatically read a file. The service will take care of
 *    splitting the file into telemetry packets of appropriate size
 *    which are stored on the heap and linked to the TM storage or downlink
 *    handler immediately.
 *    Up to three simultaneous read operations will be permitted. After
 *    an operation has finished, the operation parameters will be moved to a
 *    different list which can be used to delete the file if TM reception was
 *    successfull. A step reply will be generated periodically for
 *    large files with an interval of 5 telemetry packets.
 *  - TC[23,136]: Clear finished read operation. This service will take care
 *    of deleting the files of older automatic read operations which already
 *    have been finished. There will be options to either clear specific
 *    files or clear the whole list.
 *  - TC[23,137]: Stop the automatic read operation in one or more of the read
 *    operation slots, so another read operation can be started.
 *
 *  - TC[23,140]: Manually read from a file. The service will also only support
 *    one read operation at a time, but will implement sequence checking
 *    for consecutive read operations on the same file.
 *  - TC[23,141]: Read reply.
 *  - TC[23,142]: Stop or finish read operation. This service will stop or
 *    finish the read operation so that another read operation can be started.
 *    A telemetry packet containing the last valid sequence number, the
 *    repository and file name of the current operation will be generated.
 *  - TC[23,143]: Stop read reply.

 *
 * The file copy subservice might only be partially implemented.
 * The last byte of the operation ID might be used to specify whether a
 * file is copied inside the SD-card or to the other SD-card.
 * Service capability:
 *   - TC[23,14]: Copy a file
 *   - TC[23,16]: Suspend file copy operations
 *   - TC[23,17]: Resume file copy operations
 * @author  Jakob Meier, R. Mueller
 * @ingroup pus_services
 */
class Service23FileManagement: public CommandingServiceBase {
public:
    static constexpr uint8_t NUM_PARALLEL_COMMANDS = 15;
    static constexpr uint16_t COMMAND_TIMEOUT_SECONDS = 60;

	Service23FileManagement(object_id_t objectId, uint16_t apid,
	        uint8_t serviceId);
	virtual ~Service23FileManagement();

protected:

	/** ComandingServiceBase overrides */
	virtual ReturnValue_t isValidSubservice(uint8_t subservice) override;
	virtual ReturnValue_t getMessageQueueAndObject(uint8_t subservice,
			const uint8_t *tcData, size_t tcDataLen, MessageQueueId_t *id,
			object_id_t *objectId) override;
	virtual ReturnValue_t prepareCommand(CommandMessage* message,
			uint8_t subservice, const uint8_t *tcData, size_t tcDataLen,
			uint32_t *state, object_id_t objectId) override;
	virtual ReturnValue_t handleReply(const CommandMessage *reply,
			Command_t previousCommand, uint32_t *state,
			CommandMessage *optionalNextCommand, object_id_t objectId,
			bool *isStep) override;

private:
	ReturnValue_t checkInterfaceAndAcquireMessageQueue(
	        MessageQueueId_t* messageQueueToSet, object_id_t* objectId);


	enum Subservice {
		CREATE_FILE = 1, //!< [EXPORT] : [COMMAND] Create file
		DELETE_FILE = 2, //!< [EXPORT] : [COMMADND] Delete file

		REPORT_FILE_ATTRIBUTES = 3,
		REPORT_FILE_ATTRIBUTES_REPLY = 4,

		LOCK_FILE = 5,
		UNLOCK_FILE = 6,

		FIND_FILE = 7,
		FOUND_FILES_REPLY = 8,

		CREATE_DIRECTORY = 9, //!<  [EXPORT] : [COMMAND] Create a directory
		DELETE_DIRECTORY = 10, //!<  [EXPORT] : [COMMAND] Delete a directory

        RENAME_DIRECTORY = 11,
		REPORT_REPOSITORY = 12,
		REPORT_REPOSTIROY_REPLY = 13,

		MOVE_FILE = 15,

		/** Custom subservices */
		APPEND_TO_FILE = 130, //!< [EXPORT] : [COMMAND] Append data to file
		FINISH_APPEND_TO_FILE = 131,
		FINISH_APPEND_REPLY = 132,

		READ_FROM_FILE = 135, //!< [EXPORT] : [COMMAND] Read data from a file
		READ_REPLY = 136, //!< [EXPORT] : [REPLY] Reply of subservice 129

		CLEAR_REPOSITORY = 180, //!< [EXPORT] : [COMMAND] Clears a folder, and also deletes all contained files and folders recursively. Use with care!
	};

	ReturnValue_t addDataToStore(store_address_t* storeId, const uint8_t* tcData,
	        size_t tcDataLen);
	ReturnValue_t forwardFileSystemReply(const CommandMessage* reply,
			object_id_t objectId, Subservice subservice);
};


#endif /* MISSION_PUS_SERVICE23FILEMANAGEMENT_H_ */
