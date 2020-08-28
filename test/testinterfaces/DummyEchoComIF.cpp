#include "systemObjectList.h"

#include <test/testinterfaces/DummyEchoComIF.h>

#include <fsfw/serialize/SerializeAdapter.h>
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/tmtcservices/CommandingServiceBase.h>
#include <fsfw/tmtcpacket/pus/TmPacketStored.h>


DummyEchoComIF::DummyEchoComIF(object_id_t object_id_, bool initFunnel):
	SystemObject(object_id_) {
	if(initFunnel) {
		funnel = objectManager->get<AcceptsTelemetryIF>(objects::PUS_FUNNEL);
		if (funnel != nullptr) {
			tmQueue->setDefaultDestination(funnel->getReportReceptionQueue());
		}
		else {
			sif::info << "DummyEchoComIF: PUS funnel not created yet." << std::endl;
		}
	}
}

DummyEchoComIF::~DummyEchoComIF() {}

ReturnValue_t DummyEchoComIF::initializeInterface(CookieIF * cookie) {
	return RETURN_OK;
}

ReturnValue_t DummyEchoComIF::sendMessage(CookieIF *cookie, const uint8_t * sendData,
		size_t sendLen) {
	// DummyCookie * dummyCookie = dynamic_cast<DummyCookie*>(cookie);
    replyBuffer.reserve(sendLen);
    if(sendLen != replyBuffer.size()) {
        replyBuffer.shrink_to_fit();
    }
	memcpy(replyBuffer.data(),sendData, sendLen);
	dummyBufferSize = sendLen;
	return RETURN_OK;
}

ReturnValue_t DummyEchoComIF::getSendSuccess(CookieIF *cookie) {
	return RETURN_OK;
}

ReturnValue_t DummyEchoComIF::requestReceiveMessage(CookieIF *cookie,
		size_t requestLen) {
	// debug << "ComIF Request Receive Message" << std::endl;
	return RETURN_OK;
}

ReturnValue_t DummyEchoComIF::readReceivedMessage(CookieIF *cookie,
		uint8_t **buffer, size_t *size) {
	//DummyCookie * dummyCookie = dynamic_cast<DummyCookie*>(cookie);
	*buffer = replyBuffer.data();
	*size = dummyBufferSize;
	this->dummyBufferSize = 0;

	dummyReplyCounter ++;
	if(dummyReplyCounter == 10) {
		// add anything that needs to be read periodically by dummy handler
		dummyReplyCounter = 0;
	}
	return RETURN_OK;

}

