#include <sam9g20/tmtcbridge/TmTcSerialBridge.h>

#include <fsfw/serviceinterface/ServiceInterface.h>
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
	analyzerTask = new RingBufferAnalyzer(ringBuffer,
			AnalyzerModes::DLE_ENCODING);
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
#if FSFW_CPP_OSTREAM_ENABLED == 1
			    sif::debug << "TmTcSerialBridge::handleTc: Handling TC failed!" << std::endl;
#else
	            sif::printDebug("TmTcSerialBridge::handleTc: Handling TC failed!\n");
#endif
				return result;
			}
		}
		else if(result == RingBufferAnalyzer::POSSIBLE_PACKET_LOSS) {
			// trigger event?
#if FSFW_CPP_OSTREAM_ENABLED == 1
		    sif::debug << "TmTcSerialBridge::handleTc: Possible data loss" << std::endl;
#else
		    sif::printDebug("TmTcSerialBridge::handleTc: Possible data loss\n");
#endif
			continue;
		}
		else if(result == RingBufferAnalyzer::NO_PACKET_FOUND) {
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
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "TmTcSerialBridge::sendTm: Send error with code "
		      << static_cast<int>(result) << "on bus 0." << std::endl;
#else
		sif::printError("TmTcSerialBridge::sendTm: Send error with code &d on bus0\n", result);
#endif
	}
	// If data is being sent too fast, this delay could be used.
	// It will not block the CPU, because a context switch will be requested.
	TaskFactory::delayTask(2);
	return result;
}


