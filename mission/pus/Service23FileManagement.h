/*
 * Service23FileManagement.h
 *
 *  Created on: 09.12.2019
 *      Author: Jakob Meier
 */

#ifndef MISSION_PUS_SERVICE23FILEMANAGEMENT_H_
#define MISSION_PUS_SERVICE23FILEMANAGEMENT_H_

#include <framework/objectmanager/SystemObject.h>
#include <framework/tmtcservices/CommandingServiceBase.h>

/**
 * \brief File Management Service
 *
 * Full Documentation: ECSS-E-ST-70-41C p.403
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
 *
 * \ingroup pus_services
 */
class Service23FileManagement: public CommandingServiceBase {
public:

	Service23FileManagement(object_id_t objectId_);
	virtual ~Service23FileManagement();

protected:
	virtual ReturnValue_t isValidSubservice(uint8_t subservice);
	/**
	 * Once a TC Request is valid, the existence of the destination and interface is checked.
	 * After that, its Message Queue ID is determined for the Commanding Service Base.
	 * @param subservice
	 * @param tcData Application Data of TC Packet
	 * @param tcDataLen Application Data length
	 * @param id MessageQueueId_t is stored here
	 * @param objectId Target object ID
	 * @return
	 */
	virtual ReturnValue_t getMessageQueueAndObject(uint8_t subservice,
			const uint8_t *tcData, size_t tcDataLen, MessageQueueId_t *id,
			object_id_t *objectId);

	/**
	 * After the Message Queue and Object ID are determined,
	 * the command is prepared by using a FileSystemMessage which is sent to
	 * the target memory handler.
	 * @param message DeviceHandlerMessage
	 * @param subservice
	 * @param tcData
	 * @param tcDataLen
	 * @param state
	 * @param objectId
	 * @return
	 */
	virtual ReturnValue_t prepareCommand(CommandMessage* message,
			uint8_t subservice, const uint8_t *tcData, size_t tcDataLen,
			uint32_t *state, object_id_t objectId);

	/**
	 * Called by CSB template class and contains user specific implementation
	 * of reply handling. Refer to CSB doc for more information
	 * @param reply Command Message which contains information about the command
	 * @param previousCommand
	 * @param state
	 * @param optionalNextCommand
	 * @param objectId
	 * @param isStep Flag value to mark steps of command execution
	 * @return - @c RETURN_OK, @c EXECUTION_COMPLETE or @c NO_STEP_MESSAGE to generate TC verification success
	 *         - @c INVALID_REPLY can handle unrequested replies
	 *         - Anything else triggers a TC verification failure
	 */
	virtual ReturnValue_t handleReply(const CommandMessage *reply,
			Command_t previousCommand, uint32_t *state,
			CommandMessage *optionalNextCommand, object_id_t objectId,
			bool *isStep);

private:
	ReturnValue_t checkAndAcquireTargetID(object_id_t* objectIdToSet, const uint8_t* tcData, uint32_t tcDataLen);
	ReturnValue_t checkInterfaceAndAcquireMessageQueue(MessageQueueId_t* MessageQueueToSet, object_id_t* objectId);

	ReturnValue_t prepareDeleteFileCommand(CommandMessage *message,
					const uint8_t *tcData, uint32_t tcDataLen);

	ReturnValue_t prepareCreateDirectoryCommand(CommandMessage *message,
				const uint8_t *tcData, uint32_t tcDataLen);

	ReturnValue_t prepareDeleteDirectoryCommand(CommandMessage *message,
				const uint8_t *tcData, uint32_t tcDataLen);

	ReturnValue_t prepareWriteCommand(CommandMessage *message,
			const uint8_t *tcData, uint32_t tcDataLen);

	ReturnValue_t prepareReadCommand(CommandMessage *message,
			const uint8_t *tcData, uint32_t tcDataLen);


	enum Subservice {
		CREATE_FILE = 1, //!< [EXPORT] : [COMMAND] Create file
		DELETE_FILE = 2, //!< [EXPORT] : [COMMADND] Delete file
		CREATE_DIRECTORY = 9, //!<  [EXPORT] : [COMMAND] Create a directory
		DELETE_DIRECTORY = 10, //!<  [EXPORT] : [COMMAND] Delete a directory
		WRITE = 128, //!< [EXPORT] : [COMMAND] Write data to file
		READ= 129, //!< [EXPORT] : [COMMAND] Read data from a file
		READ_REPLY = 130 //!< [EXPORT] : [REPLY] Reply of subservice 129
	};
};


#endif /* MISSION_PUS_SERVICE23FILEMANAGEMENT_H_ */
