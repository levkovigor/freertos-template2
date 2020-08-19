/*
 * TmToTcpBridge.h
 *
 *  Created on: 03.08.2015
 *      Author: baetz
 */

#ifndef BSP_ML505_TMTC_TCP_TMTCTCPBRIDGE_H_
#define BSP_ML505_TMTC_TCP_TMTCTCPBRIDGE_H_

#ifdef ML505
#include <fsfw/ipc/MessageQueue.h>
#include <fsfw/tmtcservices/AcceptsTelemetryIF.h>
#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/storagemanager/StorageManagerIF.h>
#include <fsfw/modes/HasModesIF.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>

class TmTcTcpBridge: public AcceptsTelemetryIF,
		public ExecutableObjectIF,
		public HasReturnvaluesIF,
		public HasModesIF,
		public SystemObject {
public:
	TmTcTcpBridge(object_id_t objectId, uint32_t remoteIPAddress,
			uint16_t remoteTmPort, uint16_t remoteTcPort, bool configureAsServer, object_id_t parentId = 0);
	~TmTcTcpBridge();
	MessageQueueId_t getReportReceptionQueue(uint8_t virtualChannel = 0);
	ReturnValue_t performOperation(uint8_t opCode);
	ReturnValue_t initialize();
	MessageQueueId_t getCommandQueue() const;

protected:
	ReturnValue_t checkModeCommand(Mode_t mode, Submode_t submode,
			uint32_t *msToReachTheMode);
	void startTransition(Mode_t mode, Submode_t submode);
	void getMode(Mode_t *mode, Submode_t *submode);
	void setToExternalControl();
	void announceMode(bool recursive);
private:
	uint8_t receiveBuffer[1024];
	int tmSocket;
	int tcSocket;
	int tmSocketConnection;
	int tcSocketConnection;
	MessageQueue tmReceptionQueue;
	StorageManagerIF* tcStore;
	StorageManagerIF* tmStore;
	MessageQueue commandQueue;
	ModeHelper modeHelper;
	object_id_t parentId;
	Mode_t mode;
	Submode_t submode;
	sockaddr_in tmAddress;
	sockaddr_in tcAddress;

	bool isConnected;
	bool isServer;

	void checkCommandQueue();
	void forwardTmPacketsFromFwToTcp();
	void forwardTcPacketsFromTcpToFw();
	void clearTmQueue();
	ReturnValue_t setUpConnection();
};
#endif //ML505
#endif /* BSP_ML505_TMTC_TCP_TMTCTCPBRIDGE_H_ */
