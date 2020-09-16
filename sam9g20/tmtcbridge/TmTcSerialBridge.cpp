#include <sam9g20/tmtcbridge/TmTcSerialBridge.h>

#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/tasks/TaskFactory.h>
#include <fsfw/timemanager/Clock.h>
#include <fsfw/timemanager/Stopwatch.h>
#include <fsfw/tmtcpacket/pus/TcPacketStored.h>
#include <fsfw/globalfunctions/arrayprinter.h>
#include <fsfw/globalfunctions/DleEncoder.h>

#include <cmath>

TmTcSerialBridge::TmTcSerialBridge(object_id_t objectId,
		object_id_t tcDestination, object_id_t tmStoreId,
		object_id_t tcStoreId, object_id_t sharedRingBufferId):
		TmTcBridge(objectId, tcDestination, tmStoreId, tcStoreId),
		sharedRingBufferId(sharedRingBufferId) {
    TmTcBridge::setNumberOfSentPacketsPerCycle(5);
}

TmTcSerialBridge::~TmTcSerialBridge() {
}

ReturnValue_t TmTcSerialBridge::initialize() {
	SharedRingBuffer* ringBuffer =
			objectManager->get<SharedRingBuffer>(sharedRingBufferId);
	if(ringBuffer == nullptr) {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	analyzerTask = new SerialAnalyzerTask(ringBuffer,AnalyzerModes::DLE_ENCODING);
	return TmTcBridge::initialize();
}


ReturnValue_t TmTcSerialBridge::performOperation(uint8_t operationCode) {
	TmTcBridge::performOperation();
	return RETURN_OK;
}

ReturnValue_t TmTcSerialBridge::handleTc() {
	for(uint8_t tcPacketIdx = 0; tcPacketIdx < MAX_TC_PACKETS_HANDLED;
			tcPacketIdx++) {
		size_t packetFoundLen = 0;
		ReturnValue_t result = analyzerTask->checkForPackets(tcArray.data(),
				tcArray.size(), &packetFoundLen);
		if(result == HasReturnvaluesIF::RETURN_OK) {
			result = handleTcReception(packetFoundLen);
			if(result != HasReturnvaluesIF::RETURN_OK) {
				return result;
			}
		}
		else if(result == SerialAnalyzerTask::POSSIBLE_PACKET_LOSS) {
			// trigger event?
			continue;
		}
		else if(result == SerialAnalyzerTask::NO_PACKET_FOUND) {
			return HasReturnvaluesIF::RETURN_OK;
		}
	}

	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t TmTcSerialBridge::handleTcReception(size_t foundLen) {
	store_address_t storeId;
	ReturnValue_t result = tcStore->addData(&storeId,
			tcArray.data(), foundLen);
	if(result == HasReturnvaluesIF::RETURN_FAILED) {
		return result;
	}
	TmTcMessage tcMessage(storeId);
	return MessageQueueSenderIF::sendMessage(getRequestQueue(),
			&tcMessage);
}

ReturnValue_t TmTcSerialBridge::sendTm(const uint8_t *data, size_t dataLen) {
    //Stopwatch stopwatch;
    size_t encodedLen = 0;
    ReturnValue_t result = DleEncoder::encode(data, dataLen, tmArray.data(),
            tmArray.size(), &encodedLen, true);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }
    result = UART_write(bus0_uart, tmArray.data(), encodedLen);
	//sif::info << "TmTcSerialBridge::sendTm: Sending telemetry." << std::endl;
	if(result != RETURN_OK) {
		sif::error << "TmTcSerialBridge::sendTm: Send error with code "
		      << static_cast<int>(result) << "on bus 0." << std::endl;
	}
	// If data is being sent too fast, this delay could be used.
	// It will not block the CPU, because a context switch will be requested.
	TaskFactory::delayTask(2);
	return result;
}


