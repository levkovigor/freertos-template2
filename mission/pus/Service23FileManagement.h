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
 * A set of custom subservices will be implemented:
 *  - TC[23,130]: Append to file. The service will  implement sequence checking
 *    for consecutive append operations on the same file but only support one
 *    append operation at a time.
 *  - TC[23,131]: Stop or finish append operation. This service will stop or
 *    finish the append operation so that another append operation can be
 *    started. A telemetry packet containing the last valid sequence number,
 *    the repository and file name of the current operation will be generated.
 *  - TC[23,132]: Stop append reply.
 *
 *  - TC[23,135]: Read from a file. The service will also only support
 *    one read operation at a time, but will implement sequence checking
 *    for consecutive read operations on the same file.
 *  - TC[23,136]: Read reply.
 *  - TC[23,137]: Stop or finish read operation. This service will stop or
 *    finish the read operation so that another read operation can be started.
 *    A telemetry packet containing the last valid sequence number, the
 *    repository and file name of the current operation will be generated.
 *  - TC[23,138]: Stop read reply.
 *
 *  Actually those belong in service 8..
 *  - TC[23,150]: Select active SD-card.
 *  - TC[23,151]: Report active SD-card.
 *  - TC[23,152]: Active SD-card reply.
 *
 *  - TC[23,160]:
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

		DUMP_FILE_STRUCTURE = 131, //!< [EXPORT] : [COMMAND] Dump structure of whole SD card as ASCII file.
		DUMP_FILE_STRUCTURE_REPLY = 132, //!< [EXPORT] : [REPLY] ASCII reply if file is small enough, otherwise only repository name and filename.
		PRINT_SD_CARD = 133, //!< [EXPORT] : [COMMAND] Print SD card to console.

		/**
		 * Select the active SD card. All other tasks using SD cards will
		 * be notified as well.
		 */
		SELECT_ACTIVE_SD_CARD = 150, //!< [EXPORT] : [COMMAND] Swap the active SD card.
		REPORT_ACTIVE_SD_CARD = 151, //!< [EXPORT] : [REPLY] Reply which contains now active SD card
		ACTIVE_SD_CARD_REPLY = 152,

		/**
		 * Select the SD card which will be chosen on start-up. The value
		 * is stored in FRAM and used by all other tasks using the SD cards
		 * as well.
		 */
		SELECT_PREFERED_SD_CARD = 160, //!< [EXPORT] : [COMMAND] Select the preferred SD card which will be picked on reboot. (value stored in FRAM)
		REPORT_PREFERED_SD_CARD = 161, //!< [EXPORT] : [COMMAND] Report currently prefered SD card.
		PREFERED_SD_CARD_REPLY = 162, //!< [EXPORT] : [REPLY] Report currently prefered SD card.

		CLEAR_REPOSITORY = 180, //!< [EXPORT] : [COMMAND] Clears a folder, and also deletes all contained files and folders recursively. Use with care!
		CLEAR_SD_CARD = 181, //!< [EXPORT] : [COMMAND] Clears SD card. Use with care!
		FORMAT_SD_CARD = 182 //! [EXPORT] : [COMMAND] Formats SD card which also deletes everything. Use with care!
	};

	ReturnValue_t addDataToStore(store_address_t* storeId, const uint8_t* tcData,
	        size_t tcDataLen);
};


#endif /* MISSION_PUS_SERVICE23FILEMANAGEMENT_H_ */
