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
    TmTcBridge::setNumberOfSentPacketsPerCycle(3);
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

ReturnValue_t TmTcSerialBridge::handleTmQueue() {
    //Stopwatch stopwatch = Stopwatch(false);
    TmTcMessage message;
    const uint8_t* data = nullptr;
    size_t tmSize = 0;
//    uint16_t framePosition = 0;
//    uint8_t counter = 0;
    ReturnValue_t status = HasReturnvaluesIF::RETURN_OK;
    //std::array<uint8_t, SERIAL_FRAME_LEN> serialFrame = {};
    for (ReturnValue_t result = tmTcReceptionQueue->receiveMessage(&message);
    		result == RETURN_OK;
    		result = tmTcReceptionQueue->receiveMessage(&message))
    {
        if(communicationLinkUp == false) {
            storeDownlinkData(&message);
            continue;
        }

        result = tmStore->getData(message.getStorageId(), &data, &tmSize);
        if(result != HasReturnvaluesIF::RETURN_OK) {
        	sif::error << "TmTcSerialBridge::handleTmQueue: Invalid store ID!"
        			<< std::endl;
        	status = result;
        	continue;
        }

        result = sendTm(data, tmSize);
//        if(framePosition + tmSize < SERIAL_FRAME_LEN) {
//            std::copy(data, data + tmSize, serialFrame.data() + framePosition);
//            framePosition += tmSize;
//            result = tmStore->deleteData(message.getStorageId());
//            if(result != RETURN_OK) {
//                sif::error << "TmTcSerialBridge: Deletion of TM data not "
//                		<< "possible!" << std::endl;
//            }
//        }
//        else {
//        	storeDownlinkData(&message);
//        	continue;
//        }
//        counter ++;
//    }
//
//    if(counter > 0) {
//
    }
    return status;
}


ReturnValue_t TmTcSerialBridge::handleStoredTm() {
    ReturnValue_t result = RETURN_OK;
    const uint8_t* data = nullptr;
    size_t tmSize = 0;
//    while(not tmFifo.empty() && counter < sentPacketsPerCycle) {
//        //info << "TMTC Bridge: Sending stored TM data. There are "
//        //     << (int) fifo.size() << " left to send\r\n" << std::flush;
//        store_address_t storeId;
//
//        tmFifo.peek(&storeId);
//        result = tmStore->getData(storeId, &data, &tmSize);
//        if(result != HasReturnvaluesIF::RETURN_OK) {
//        	tmFifo.pop();
//        	sif::error << "TmTcSerialBridge::handleStoredTm: Invalid store "
//        			"ID!" << std::endl;
//        	continue;
//        }
//
//        if(framePosition + tmSize < SERIAL_FRAME_LEN) {
//            std::copy(data, data + tmSize, serialFrame.data() + framePosition);
//            framePosition += tmSize;
//            result = tmStore->deleteData(storeId);
//            if(result != RETURN_OK) {
//                sif::error << "TmTcSerialBridge: Deletion of TM data not possible!"
//                        << std::endl;
//            }
//            tmFifo.pop();
//        }
//        else {
//            break;
//        }
//        counter ++;
//
//        if(tmFifo.empty()) {
//            tmStored = false;
//        }
//    }

//    if (counter > 0) {
        //info << "TmTcSerialBridge: Sending " << static_cast<int>(counter)
        //     << " stored packets" << std::endl;
//        result = sendTm(), SERIAL_FRAME_LEN);
//        if(result != RETURN_OK) {
//            sif::error << "TMTC Bridge: Could not send stored downlink data"
//                  << std::endl;
//        }
//   }
    return result;
}

ReturnValue_t TmTcSerialBridge::sendTm(const uint8_t *data, size_t dataLen) {
    size_t encodedLen = 0;
    ReturnValue_t result = DleEncoder::encode(data, dataLen, tmArray.data(),
            tmArray.size(), &encodedLen, true);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }
    //arrayprinter::print(tmArray.data(), encodedLen);
    result = UART_write(bus0_uart, tmArray.data(), encodedLen);
	//sif::info << "TmTcSerialBridge::sendTm: Sending telemetry." << std::endl;
	if(result != RETURN_OK) {
		sif::error << "Serial Bridge: Send error with code "
		      << (int) result << "on bus 0." << std::endl;
	}
	TaskFactory::delayTask(1);
	return result;
}


