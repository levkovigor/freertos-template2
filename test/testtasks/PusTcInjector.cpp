#include <test/testtasks/PusTcInjector.h>

#include <fsfw/objectmanager/ObjectManagerIF.h>
#include <fsfw/tmtcservices/AcceptsTelecommandsIF.h>
#include <fsfw/tmtcservices/TmTcMessage.h>
#include <fsfw/tmtcpacket/pus/TcPacketBase.h>
#include <fsfw/ipc/QueueFactory.h>
#include <fsfw/globalfunctions/arrayprinter.h>
#include <fsfw/tmtcpacket/pus/TcPacketStored.h>
#include <fsfw/serviceinterface/ServiceInterface.h>

PusTcInjector::PusTcInjector(object_id_t objectId, object_id_t destination,
		object_id_t tcStore, uint16_t defaultApid): SystemObject(objectId),
		defaultApid(defaultApid), destination(destination), tcStoreId(tcStore) {
}

PusTcInjector::~PusTcInjector() {
}

ReturnValue_t PusTcInjector::injectPusTelecommand(uint8_t service,
		uint8_t subservice,const uint8_t* appData, size_t appDataLen) {
	return injectPusTelecommand(service, subservice, defaultApid, appData,
			appDataLen);
}

// TODO: ACK flags
ReturnValue_t PusTcInjector::injectPusTelecommand(uint8_t service,
		uint8_t subservice,uint16_t apid, const uint8_t* appData,
		size_t appDataLen) {
	// Prepare TC packet. Store into TC store immediately.
	TcPacketStored tcPacket(apid, service, subservice, sequenceCount++);

	const uint8_t* packetPtr = nullptr;
	size_t packetSize = 0;
	tcPacket.getData(&packetPtr, &packetSize);
	//arrayprinter::print(packetPtr, packetSize, OutputType::BIN);

	// Send TC packet.
	TmTcMessage tcMessage(tcPacket.getStoreAddress());
	ReturnValue_t result = injectionQueue->sendToDefault(&tcMessage);
	if(result != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::warning << "PusTcInjector: Sending TMTC message failed!" << std::endl;
#else
		sif::printWarning("PusTcInjector: Sending TMTC message failed!\n");
#endif
	}
	return result;
}

ReturnValue_t PusTcInjector::initialize() {
	// Prepare message queue which is used to send telecommands.
	injectionQueue = QueueFactory::instance()->
			createMessageQueue(INJECTION_QUEUE_DEPTH);
	AcceptsTelecommandsIF* targetQueue = objectManager->
			get<AcceptsTelecommandsIF>(destination);
	if(targetQueue == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "PusTcInjector: CCSDS distributor not initialized yet!" << std::endl;
#else
		sif::printError("PusTcInjector: CCSDS distributor not initialized yet!\n");
#endif
		return ObjectManagerIF::CHILD_INIT_FAILED;
	}
	else {
		injectionQueue->setDefaultDestination(targetQueue->getRequestQueue());
	}

	// Prepare store used to store TC messages
	tcStore = objectManager->get<StorageManagerIF>(tcStoreId);
	if(tcStore == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "PusTcInjector: TC Store not initialized!" << std::endl;
#else
		sif::printError("PusTcInjector: TC Store not initialized!\n");
#endif
		return ObjectManagerIF::CHILD_INIT_FAILED;
	}
	return HasReturnvaluesIF::RETURN_OK;
}
