#include <sam9g20/tmtcbridge/TmTcSerialBridge.h>

#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/timemanager/Clock.h>
#include <fsfw/timemanager/Stopwatch.h>
#include <fsfw/tmtcpacket/pus/TcPacketStored.h>

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

	ReturnValue_t result = TmTcBridge::initialize();
	if (result != RETURN_OK) {
		sif::error << "Serial Bridge: Init error." << std::endl;
	}

	return result;
}


ReturnValue_t TmTcSerialBridge::performOperation(uint8_t operationCode) {
	TmTcBridge::performOperation();
	return RETURN_OK;
}

ReturnValue_t TmTcSerialBridge::handleTc() {
	size_t packetFoundLen = 0;

	ReturnValue_t result = analyzerTask->checkForPackets(tcArray.data(),
						TC_FRAME_MAX_LEN + 10, &packetFoundLen);
	if(result == SerialAnalyzerTask::NO_PACKET_FOUND) {
		return HasReturnvaluesIF::RETURN_OK;
	}
	else if(result == SerialAnalyzerTask::POSSIBLE_PACKET_LOSS) {
		// trigger event
	}
	else if(result == HasReturnvaluesIF::RETURN_OK) {
		result = handleTcReception(packetFoundLen);
		if(result != HasReturnvaluesIF::RETURN_OK) {
			sif::error << "TmTcSerialBridge::handleTc: TC reception failed!"
					<< std::endl;
		}
	}

	for(uint8_t tcPacketIdx = 0; tcPacketIdx < MAX_TC_PACKETS_HANDLED;
			tcPacketIdx++) {
		ReturnValue_t result = analyzerTask->checkForPackets(tcArray.data(),
				TC_FRAME_MAX_LEN + 10, &packetFoundLen);
		if(result == HasReturnvaluesIF::RETURN_OK) {
			continue;
		}
		else if(result == SerialAnalyzerTask::POSSIBLE_PACKET_LOSS) {
			// trigger event
			continue;
		}
		else {
			return result;
		}
	}

	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t TmTcSerialBridge::handleTcReception(size_t foundLen) {
	//TcPacketStored tcPacket(tcArray.data(), packetFoundLen);
	store_address_t storeId;
	ReturnValue_t result = tcStore->addData(&storeId,
			tcArray.data(), foundLen);
	if(result != HasReturnvaluesIF::RETURN_FAILED) {
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
    uint16_t framePosition = 0;
    uint8_t counter = 0;
    std::array<uint8_t, SERIAL_FRAME_LEN> serialFrame = {};
    for (ReturnValue_t result = tmTcReceptionQueue->receiveMessage(&message);
         result == RETURN_OK;
         result = tmTcReceptionQueue->receiveMessage(&message))
    {
        if(communicationLinkUp == false) {
            result = storeDownlinkData(&message);
            return result;
        }

        result = tmStore->getData(message.getStorageId(), &data, &tmSize);

        if(framePosition + tmSize < SERIAL_FRAME_LEN) {
            std::copy(data, data + tmSize, serialFrame.data() + framePosition);
            framePosition += tmSize;
            result = tmStore->deleteData(message.getStorageId());
            if(result != RETURN_OK) {
                sif::error << "TmTcSerialBridge: Deletion of TM data not possible!"
                      << std::endl;
            }
        }
        else {
            tmFifo.insert(message.getStorageId());
            break;
        }
        counter ++;
    }

    if(counter > 0) {
        //stopwatch.displayOnDestruction = true;
        //info << "TmTcSerialBridge: Sending "
        //     << static_cast<int>(counter) << " packets" << std::endl;
        return sendTm(serialFrame.data(), SERIAL_FRAME_LEN);
    }
    return RETURN_OK;
}


ReturnValue_t TmTcSerialBridge::handleStoredTm() {
    uint8_t counter = 0;
    ReturnValue_t result = RETURN_OK;
    uint16_t framePosition = 0;
    const uint8_t* data = nullptr;
    size_t tmSize = 0;
    std::array<uint8_t, SERIAL_FRAME_LEN> serialFrame = {};
    while(not tmFifo.empty() && counter < sentPacketsPerCycle) {
        //info << "TMTC Bridge: Sending stored TM data. There are "
        //     << (int) fifo.size() << " left to send\r\n" << std::flush;
        store_address_t storeId;

        tmFifo.peek(&storeId);
        result = tmStore->getData(storeId, &data, &tmSize);
        if(framePosition + tmSize < SERIAL_FRAME_LEN) {
            std::copy(data, data + tmSize, serialFrame.data() + framePosition);
            framePosition += tmSize;
            result = tmStore->deleteData(storeId);
            if(result != RETURN_OK) {
                sif::error << "TmTcSerialBridge: Deletion of TM data not possible!"
                        << std::endl;
            }
            tmFifo.pop();
        }
        else {
            break;
        }
        counter ++;

        if(tmFifo.empty()) {
            tmStored = false;
        }
    }

    if (counter > 0) {
        //info << "TmTcSerialBridge: Sending " << static_cast<int>(counter)
        //     << " stored packets" << std::endl;
        result = sendTm(serialFrame.data(), SERIAL_FRAME_LEN);
        if(result != RETURN_OK) {
            sif::error << "TMTC Bridge: Could not send stored downlink data"
                  << std::endl;
        }
    }
    return result;
}

ReturnValue_t TmTcSerialBridge::sendTm(const uint8_t *data, size_t dataLen) {
    ReturnValue_t result = RETURN_OK;
    result = UART_write(bus0_uart, const_cast<uint8_t *>(data), dataLen);
	//info << "Serial Bridge: Sending telemetry." << std::endl;
	if(result != RETURN_OK) {
		sif::error << "Serial Bridge: Send error with code "
		      << (int) result << "on bus 0." << std::endl;
	}
	return result;
}


