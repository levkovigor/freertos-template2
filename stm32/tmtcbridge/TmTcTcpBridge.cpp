/*
 * TmToTcpBridge.cpp
 *
 *  Created on: 03.08.2015
 *      Author: baetz
 */

#ifdef ML505

#include <fsfw/tmtcservices/TmTcMessage.h>
#include <netinet/in.h>
#include <fsfw/objectmanager/ObjectManagerIF.h>
#include <fsfw/tmtcpacket/SpacePacketBase.h>
#include <fsfw/tmtcservices/AcceptsTelecommandsIF.h>
#include <bsp_ml505/tmtc_tcp/TmTcTcpBridge.h>
#include <stdio.h>
#include <fsfw/subsystem/SubsystemBase.h>
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fcntl.h>

TmTcTcpBridge::TmTcTcpBridge(object_id_t objectId, uint32_t remoteIPAddress,
		uint16_t remoteTmPort, uint16_t remoteTcPort, bool configureAsServer, object_id_t parentId) :
		SystemObject(objectId), tmSocket(0), tcSocket(0), tmSocketConnection(0), tcSocketConnection(
				0), tmReceptionQueue(60), tcStore(
		NULL), tmStore(NULL), modeHelper(this), parentId(parentId), mode(
				MODE_OFF), submode(SUBMODE_NONE), isConnected(false), isServer(configureAsServer) {

	memset((uint8_t*) &tmAddress, 0, sizeof(tmAddress));
	tmAddress.sin_family = AF_INET;
	tmAddress.sin_addr.s_addr = htonl(remoteIPAddress);
	tmAddress.sin_port = htons(remoteTmPort);

	memset((uint8_t*) &tcAddress, 0, sizeof(tcAddress));
	tcAddress.sin_family = AF_INET;
	tcAddress.sin_addr.s_addr = htonl(remoteIPAddress);
	tcAddress.sin_port = htons(remoteTcPort);


}

TmTcTcpBridge::~TmTcTcpBridge() {
	if (tcSocket > 0) {
		close(tcSocket);
	}
	if (tcSocketConnection > 0) {
		close(tcSocketConnection);
	}
	if (tmSocket > 0) {
		close(tmSocket);
	}
	if (tmSocketConnection > 0) {
		close(tmSocketConnection);
	}
}

MessageQueueId_t TmTcTcpBridge::getReportReceptionQueue(
		uint8_t virtualChannel) {
	//Ignore VC's. If necessary, we may set up different connections for different VC's.
	return tmReceptionQueue.getId();
}

ReturnValue_t TmTcTcpBridge::performOperation(uint8_t opCode) {
	//We're the client and would like to set up a connection.
	//Connect should block the task if no connection is set up.
	checkCommandQueue();
	if (!isConnected) {
		ReturnValue_t result = setUpConnection();
		if (result == HasReturnvaluesIF::RETURN_OK) {
			debug << "TCP TM/TC connection established.\n";
			isConnected = true;
		} else {
			clearTmQueue();
		}
		return RETURN_OK;
	}
	//If CCSDS Assembly is passive, we don't forward TM.
	if (submode == SUBMODE_NONE) {
		clearTmQueue();
	} else {
		forwardTmPacketsFromFwToTcp();
	}
	forwardTcPacketsFromTcpToFw();
	return RETURN_OK;
}

ReturnValue_t TmTcTcpBridge::initialize() {
	tcStore = objectManager->get<StorageManagerIF>(objects::TC_STORE);
	if (tcStore == NULL) {
		return RETURN_FAILED;
	}
	tmStore = objectManager->get<StorageManagerIF>(objects::TM_STORE);
	if (tmStore == NULL) {
		return RETURN_FAILED;
	}
	AcceptsTelecommandsIF* tcDistributor = objectManager->get<
			AcceptsTelecommandsIF>(objects::CCSDS_PACKET_DISTRIBUTOR);
	if (tcDistributor == NULL) {
		return RETURN_FAILED;
	}
	tmReceptionQueue.setDefaultDestination(tcDistributor->getRequestQueue());

	if (parentId != 0) {
		SubsystemBase *parent = objectManager->get<SubsystemBase>(parentId);
		if (parent == NULL) {
			return RETURN_FAILED;
		}
		MessageQueueId_t parentQueue = parent->getCommandQueue();
		ReturnValue_t result = parent->registerChild(getObjectId());
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
		result = modeHelper.initialize(parentQueue);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
	}

	//Initialize connections
	sockaddr_in address;
	memset((uint8_t*) &address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = 0;

	//Create and bind tmSocket

	if (isServer) {
		debug << "Configuring as Server." << std::endl;
		tmAddress.sin_addr.s_addr = htonl(INADDR_ANY);
		tmSocket = socket(PF_INET, SOCK_STREAM, 0);
		if (tmSocket < 0) {
			debug << "Error opening TC socket!\n";
			return RETURN_FAILED;
		}
		int flags = fcntl(tmSocket, F_GETFL, 0);
		if (flags < 0) {
			debug << "Error reading flags\n";
			return RETURN_FAILED;
		}
		flags |= O_NONBLOCK;
		if (fcntl(tmSocket, F_SETFL, flags) != 0) {
			debug << "Error setting to non-blocking mode.\n";
			return RETURN_FAILED;
		}
		if (bind(tmSocket, (sockaddr*) &tmAddress, sizeof(address)) < 0) {
			debug << "Error binding TM socket!\n";
			return RETURN_FAILED;
		}
		if (listen(tmSocket, 1) < 0) {
			debug << "Error listening to TM socket!\n";
		}
	} else {
		debug << "Configuring as Client." << std::endl;
		tmSocketConnection = socket(PF_INET, SOCK_STREAM, 0);
		if (tmSocketConnection < 0) {
			debug << "Error opening TM socket!\n";
			return RETURN_FAILED;
		}
		if (bind(tmSocketConnection, (sockaddr*) &address, sizeof(address)) < 0) {
			debug << "Error binding TM socket!\n";
			return RETURN_FAILED;
		}
	}
	//Create and bind tcSocket

	if (isServer) {
		tcAddress.sin_addr.s_addr = htonl(INADDR_ANY);
		tcSocket = socket(PF_INET, SOCK_STREAM, 0);
		if (tcSocket < 0) {
			debug << "Error opening TC socket!\n";
			return RETURN_FAILED;
		}
		if (bind(tcSocket, (sockaddr*) &tcAddress, sizeof(address)) < 0) {
			debug << "Error binding TC socket!\n";
			return RETURN_FAILED;
		}
		if (listen(tcSocket, 1) < 0) {
			debug << "Error listening to TC socket!\n";
		}
	} else {
		tcSocketConnection = socket(PF_INET, SOCK_STREAM, 0);
		if (tcSocketConnection < 0) {
			debug << "Error opening TC socket!\n";
			return RETURN_FAILED;
		}
		if (bind(tcSocketConnection, (sockaddr*) &address, sizeof(address)) < 0) {
			debug << "Error binding TC socket!\n";
			return RETURN_FAILED;
		}
	}
	return RETURN_OK;
}

void TmTcTcpBridge::forwardTmPacketsFromFwToTcp() {
	TmTcMessage message;
	const uint8_t* data = NULL;
	uint32_t size = 0;
	uint32_t writtenBytes = 0;
	for (ReturnValue_t result = tmReceptionQueue.receiveMessage(&message);
			result == RETURN_OK;
			result = tmReceptionQueue.receiveMessage(&message)) {
		ReturnValue_t result = tmStore->getData(message.getStorageId(), &data,
				&size);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			continue;
		}
		writtenBytes = 0;
		while (writtenBytes < size) {
			uint32_t writeResult = write(tmSocketConnection, data,
					(size - writtenBytes));
			if (writeResult >= 0) {
				writtenBytes += writeResult;
			} else {
				debug << "Error writing!\n";
				tmStore->deleteData(message.getStorageId());
				break;
			}
		}
		tmStore->deleteData(message.getStorageId());
	}
}

MessageQueueId_t TmTcTcpBridge::getCommandQueue() const {
	return commandQueue.getId();
}

ReturnValue_t TmTcTcpBridge::checkModeCommand(Mode_t mode, Submode_t submode,
		uint32_t* msToReachTheMode) {
	return RETURN_OK;
}

void TmTcTcpBridge::startTransition(Mode_t mode, Submode_t submode) {
	this->mode = mode;
	this->submode = submode;
	modeHelper.modeChanged(mode, submode);
}

void TmTcTcpBridge::getMode(Mode_t* mode, Submode_t* submode) {
	*mode = this->mode;
	*submode = this->submode;
}

void TmTcTcpBridge::setToExternalControl() {
	return;
}

void TmTcTcpBridge::announceMode(bool recursive) {
	triggerEvent(MODE_INFO, mode, submode);
}

void TmTcTcpBridge::checkCommandQueue() {
	CommandMessage message;
	for (ReturnValue_t result = commandQueue.receiveMessage(&message);
			result == RETURN_OK;
			result = commandQueue.receiveMessage(&message)) {
		result = modeHelper.handleModeCommand(&message);
		if (result == RETURN_OK) {
			continue;
		}
		if (result != RETURN_OK) {
			message.clearCommandMessage();
			CommandMessage reply(CommandMessage::REPLY_REJECTED,
					CommandMessage::UNKNOW_COMMAND, 0);
			commandQueue.reply(&reply);
		}
	}
}

//TODO: I think this crashes badly if we receive broken packets, and the performance is poor (one TC/cycle), but for now it's ok.
void TmTcTcpBridge::forwardTcPacketsFromTcpToFw() {
	//Ok, try to read header first and then body of SpacePacket.
	uint32_t bytesReceived = 0;
	int32_t result = 0;
	while (bytesReceived < sizeof(CCSDSPrimaryHeader)) {
		result = read(tcSocketConnection, receiveBuffer,
				sizeof(CCSDSPrimaryHeader) - bytesReceived);
		if (result >= 0) {
			bytesReceived += result;
		} else {
			//Nothing to get at the moment.
			return;
		}
	}
	SpacePacketBase packet(receiveBuffer);
	bytesReceived = 0;
	uint32_t expectedData = packet.getPacketDataLength() + 1;
	while (bytesReceived < expectedData) {
		result = read(tcSocketConnection,
				receiveBuffer + sizeof(CCSDSPrimaryHeader) + bytesReceived,
				expectedData - bytesReceived);
		if (result >= 0) {
			bytesReceived += result;
		} else {
			//TODO: Good idea to loop?
		}
	}
	//We should have the full packet in the receive buffer.
	store_address_t storeId;
	ReturnValue_t returnValue = tcStore->addData(&storeId, receiveBuffer,
			packet.getFullSize());
	if (returnValue != RETURN_OK) {
		return;
	}
	TmTcMessage message(storeId);
	if (tmReceptionQueue.sendToDefault(&message) != RETURN_OK) {
		tcStore->deleteData(storeId);
	}
}

void TmTcTcpBridge::clearTmQueue() {
	TmTcMessage message;
	for (ReturnValue_t result = tmReceptionQueue.receiveMessage(&message);
			result == RETURN_OK;
			result = tmReceptionQueue.receiveMessage(&message)) {
		tmStore->deleteData(message.getStorageId());
	}
}

ReturnValue_t TmTcTcpBridge::setUpConnection() {
	//Set up TM socket
	if (isServer) {
		socklen_t socketAddressLength = sizeof(tmAddress);
		tmSocketConnection = accept(tmSocket, (struct sockaddr *) &tmAddress,
				&socketAddressLength);
		if (tmSocketConnection < 0) {
			//Better fail silently.
			return RETURN_FAILED;
		}
	} else {
		if (connect(tmSocketConnection, (struct sockaddr *) &tmAddress,
				sizeof(tmAddress)) < 0) {
			//Better fail silently.
			return RETURN_FAILED;
		}
	}
	timeval timeout = { 0, 20 };
	if (setsockopt(tmSocketConnection, SOL_SOCKET, SO_RCVTIMEO, &timeout,
			sizeof(timeout)) < 0) {
		debug << "Error setting SO_RCVTIMEO socket options!\n";
		return RETURN_FAILED;
	}
	if (setsockopt(tmSocketConnection, SOL_SOCKET, SO_SNDTIMEO, &timeout,
			sizeof(timeout)) < 0) {
		debug << "Error setting SO_SNDTIMEO socket options!\n";
		return RETURN_FAILED;
	}
	//Set up TC socket
	if (isServer) {
		socklen_t socketAddressLength = sizeof(tcAddress);
		tcSocketConnection = accept(tcSocket, (struct sockaddr *) &tcAddress,
				&socketAddressLength);
		if (tcSocketConnection < 0) {
			//Better fail silently.
			return RETURN_FAILED;
		}
	} else {
		if (connect(tcSocketConnection, (struct sockaddr *) &tcAddress,
				sizeof(tcAddress)) < 0) {
			//Better fail silently.
			return RETURN_FAILED;
		}
	}

	if (setsockopt(tcSocketConnection, SOL_SOCKET, SO_RCVTIMEO, &timeout,
			sizeof(timeout)) < 0) {
		debug << "Error setting SO_RCVTIMEO socket options!\n";
		return RETURN_FAILED;
	}
	if (setsockopt(tcSocketConnection, SOL_SOCKET, SO_SNDTIMEO, &timeout,
			sizeof(timeout)) < 0) {
		debug << "Error setting SO_SNDTIMEO socket options!\n";
		return RETURN_FAILED;
	}
	return RETURN_OK;
}

#endif //ML505
