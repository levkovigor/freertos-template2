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
		CREATE_DIRECTORY = 9, //!<  [EXPORT] : [COMMAND] Create a directory
		DELETE_DIRECTORY = 10, //!<  [EXPORT] : [COMMAND] Delete a directory
		APPEND_TO_FILE = 128, //!< [EXPORT] : [COMMAND] Append data to file
		READ_FROM_FILE = 129, //!< [EXPORT] : [COMMAND] Read data from a file
		READ_REPLY = 130 //!< [EXPORT] : [REPLY] Reply of subservice 129
	};

	ReturnValue_t addDataToStore(store_address_t* storeId, const uint8_t* tcData,
	        size_t tcDataLen);
};


#endif /* MISSION_PUS_SERVICE23FILEMANAGEMENT_H_ */
