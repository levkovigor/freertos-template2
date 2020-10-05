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
 * systems and files. Not all subservices listed below are yet implemented.
 * Service capability:
 *   - TC[23,1]: Create a file
 *   - TC[23,2]: Delete a file
 *   - TC[23,3]: Report the attributes of a file
 *   - TM[23,4]: Response of TC[23,3]
 *   - TC[23,5]: Lock a file
 *   - TC[23,6]: Unlock a file
 *   - TC[23,7]: Find files
 *   - TM[23,8]: Response of TC[23,7]
 *   - TC[23,9]: Create a directory
 *   - TC[23,10]: Delete a directory
 *   - TC[23,11]: Rename a directory
 *   - TC[23,12]: Summary-report the content of a repository
 *   - TM[23,13]: Response of TC[23,12]
 *   - TC[23,14]: Copy a file
 *   - TC[23,15]: Move a file
 *   - TC[23,16]: Suspend file copy operations
 *   - TC[23,17]: Resume file copy operations
 * @author  Jakob Meier
 * @ingroup pus_services
 */
class Service23FileManagement: public CommandingServiceBase {
public:
    static constexpr uint8_t NUM_PARALLEL_COMMANDS = 4;
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

		REPORT_REPOSITORY = 12,
		REPORT_REPOSTIROY_REPLY = 13,

		RENAME_DIRECTORY = 14,

		/** Custom subservices */
		APPEND_TO_FILE = 130, //!< [EXPORT] : [COMMAND] Append data to file
		READ_FROM_FILE = 131, //!< [EXPORT] : [COMMAND] Read data from a file
		READ_REPLY = 132, //!< [EXPORT] : [REPLY] Reply of subservice 129

		DUMP_FILE_STRUCTURE = 133, //!< [EXPORT] : [COMMAND] Dump structure of whole SD card as ASCII file.
		DUMP_FILE_STRUCTURE_REPLY = 134, //!< [EXPORT] : [REPLY] ASCII reply if file is small enough, otherwise only repository name and filename.
		PRINT_SD_CARD = 135, //!< [EXPORT] : [COMMAND] Print SD card to console.

		SELECT_ACTIVE_SD_CARD = 150, //!< [EXPORT] : [COMMAND] Swap the active SD card.
		REPORT_ACTIVE_SD_CARD = 151, //!< [EXPORT] : [REPLY] Reply which contains now active SD card
		ACTIVE_SD_CARD_REPLY = 152,

		SELECT_PREFERRED_SD_CARD = 153, //!< [EXPORT] : [COMMAND] Select the preferred SD card which will be picked on reboot. (value stored in FRAM)
		REPORT_PREFERRED_SD_CARD = 154, //!< [EXPORT] : [COMMAND] Report currently prefered SD card.
		PREFERRED_SD_CARD_REPLY = 155, //!< [EXPORT] : [REPLY] Report currently prefered SD card.

		CLEAR_REPOSITORY = 180, //!< [EXPORT] : [COMMAND] Clears a folder, and also deletes all contained files and folders recursively. Use with care!
		CLEAR_SD_CARD = 181 //!< [EXPORT] : [COMMAND] Clears SD card. Use with care!
	};

	ReturnValue_t addDataToStore(store_address_t* storeId, const uint8_t* tcData,
	        size_t tcDataLen);
};


#endif /* MISSION_PUS_SERVICE23FILEMANAGEMENT_H_ */
