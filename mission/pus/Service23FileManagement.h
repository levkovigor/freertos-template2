/*
 * Service23FileManagement.h
 *
 *  Created on: 09.12.2019
 *      Author: Jakob Meier
 */

#ifndef MISSION_PUS_SERVICE23FILEMANAGEMENT_H_
#define MISSION_PUS_SERVICE23FILEMANAGEMENT_H_

#include <fsfw/tmtcservices/PusServiceBase.h>
#include <config/objects/systemObjectList.h>

/**
 * \brief File Management Service
 *
 * Full Documentation: ECSS-E-ST-70-41C p.403
 *
 * The file management service provides the capability to manage on-board file
 * systems and files
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
class Service23FileManagement: public PusServiceBase {
public:
	// Custom events which can be triggered
	static const uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::PUS_SERVICE_23;
	static const Event TEST2 = MAKE_EVENT(0, SEVERITY::INFO);

	enum Subservice {
		CREATE_FILE = 1,
		DELETE_FILE = 2,
		REPORT_ATTRIBUTES = 3
	};

	Service23FileManagement(object_id_t objectId_);
	virtual ~Service23FileManagement();
	ReturnValue_t handleRequest(uint8_t subservice) override;
	ReturnValue_t performService() override;
private:
	uint16_t packetSubCounter;
	object_id_t fileSystemHandler = objects::SD_CARD_HANDLER;
	MessageQueueId_t fileSystemHandlerQueueId;
	MessageQueueIF* commandQueue;
	uint32_t commandQueueDepth = 20;
	StorageManagerIF *IPCStore;

};


#endif /* MISSION_PUS_SERVICE23FILEMANAGEMENT_H_ */
