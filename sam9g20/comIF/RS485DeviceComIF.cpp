#include <sam9g20/comIF/RS485DeviceComIF.h>
#include <fsfw/tasks/TaskFactory.h>
#include <sam9g20/comIF/cookies/RS485Cookie.h>
#include <mission/utility/USLPTransferFrame.h>


extern "C" {
#include <hal/Drivers/UART.h>
	}


RS485DeviceComIF::RS485DeviceComIF(object_id_t objectId, object_id_t sharedRingBufferId):
		SystemObject(objectId),
		sharedRingBufferId(sharedRingBufferId){

}
RS485DeviceComIF::~RS485DeviceComIF() {
}

ReturnValue_t RS485DeviceComIF::initialize() {

//	uartTransferFPGA1.writeData = reinterpret_cast< unsigned char *>(const_cast<char*>("FPGA1I"));
	transferFrameFPGA = new USLPTransferFrame(transmitBufferFPGA.data(), config::RS485_COM_FPGA_TFDZ_SIZE);
	transferFrameFPGA->setVersionNumber(12);
	transferFrameFPGA->setSpacecraftId(0xAFFE);
	transferFrameFPGA->setSourceFlag(true);
	transferFrameFPGA->setVirtualChannelId(3);
	transferFrameFPGA->setMapId(1);
	transferFrameFPGA->setTruncatedFlag(true);
	transferFrameFPGA->setTFDZConstructionRules(0);
	transferFrameFPGA->setProtocolIdentifier(0);
	transferFrameFPGA->setFirstHeaderOffset(0);

	sendArray[0] = nullptr;


	SharedRingBuffer* ringBuffer =
			objectManager->get<SharedRingBuffer>(sharedRingBufferId);
	if(ringBuffer == nullptr) {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	analyzerTask = new RingBufferAnalyzer(ringBuffer,
			AnalyzerModes::DLE_ENCODING);

    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t RS485DeviceComIF::initializeInterface(CookieIF *cookie) {
    return HasReturnvaluesIF::RETURN_OK;
}



ReturnValue_t RS485DeviceComIF::performOperation(uint8_t opCode) {
    RS485Devices step = static_cast<RS485Devices>(opCode);



    // check state of UART driver first, should be idle!
    // Returnvalue is ignored for now
//    checkDriverState(&retryCount);

    // Activate transceiver via GPIO
    switch(step) {
    case(RS485Devices::COM_FPGA): {
    	// Check which FPGA is active (should probably be set via DeviceHandler)
    	sif::info << "Sending to FPGA 1" << std::endl;
        if (sendArray[step] != nullptr){
        	RS485Cookie * rs485Cookie = dynamic_cast<RS485Cookie *> (sendArray[step]);
        	(void) std::memcpy(transferFrameFPGA->getDataZone(), rs485Cookie->getWriteData(), rs485Cookie->getSendLen());

        	int retval = UART_write(bus2_uart, transmitBufferFPGA.data(), transmitBufferFPGA.size());

        	rs485Cookie->setReturnValue(retval);
        	if(retval != 0){
        		rs485Cookie->setComStatus(ComStatusRS485::FAULTY);
        	}
        	else{
        		rs485Cookie->setComStatus(ComStatusRS485::TRANSFER_SUCCESS);
        	}
        	sendArray[step] = nullptr;
        }

        break;
    }
    case(RS485Devices::PCDU_VORAGO): {
    	sif::info << "Sending to PCDU" << std::endl;
        break;
    }
    case(RS485Devices::PL_VORAGO): {
    	sif::info << "Sending to PL_VORAGO" << std::endl;
        break;
    }
    case(RS485Devices::PL_PIC24): {
    	sif::info << "Sending to PL_PIC24" << std::endl;
        break;
    }
    default: {
        // should not happen
        break;
    }
    }

      //Reception
//    sif::info << "Handling Receive Buffer" << std::endl;
//    handleReceiveBuffer();


    // printout and event.
    if(retryCount > 0) {
#ifdef DEBUG
        sif::error << "RS485DeviceComIF::performOperation: RS485DeviceComIF"
                << " driver was busy for " << (uint16_t) retryCount
                << " attempts!" << std::endl;
#endif
        retryCount = 0;
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t RS485DeviceComIF::sendMessage(CookieIF *cookie,
        const uint8_t *sendData, size_t sendLen) {

	RS485Cookie * rs485Cookie = dynamic_cast<RS485Cookie *> (cookie);
	RS485Devices device = rs485Cookie->getDevice();

	// Check if there already is a message that has not been processed yet
	// We could make this condition ComStatusRS485::IDLE, but like this,
	// the user can choose to skip getSendSuccess and just queue the next message
	if(sendArray[device] == nullptr){
	rs485Cookie->setWriteData(const_cast<uint8_t*>(sendData));
	rs485Cookie->setSendLen(sendLen);
	rs485Cookie->setComStatus(ComStatusRS485::TRANSFER_INIT_SUCCESS);
	rs485Cookie->setReturnValue(0);

	sendArray[device] = cookie;
    	return HasReturnvaluesIF::RETURN_OK;
	}
	else{
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

ReturnValue_t RS485DeviceComIF::getSendSuccess(CookieIF *cookie) {

	RS485Cookie * rs485Cookie = dynamic_cast<RS485Cookie *> (cookie);

	if(rs485Cookie->getComStatus() == ComStatusRS485::TRANSFER_SUCCESS){
		rs485Cookie->setComStatus(ComStatusRS485::IDLE);
		return HasReturnvaluesIF::RETURN_OK;
	}
	else{
		// Generate event corresponding to error code stored in Cookie here
		rs485Cookie->setComStatus(ComStatusRS485::IDLE);
		return HasReturnvaluesIF::RETURN_FAILED;
	}

}

ReturnValue_t RS485DeviceComIF::requestReceiveMessage(CookieIF *cookie,
        size_t requestLen) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t RS485DeviceComIF::readReceivedMessage(CookieIF *cookie,
        uint8_t **buffer, size_t *size) {
    return HasReturnvaluesIF::RETURN_OK;
}










ReturnValue_t RS485DeviceComIF::handleReceiveBuffer() {
	for(uint8_t tcPacketIdx = 0; tcPacketIdx < MAX_TC_PACKETS_HANDLED;
			tcPacketIdx++) {
		size_t packetFoundLen = 0;
		ReturnValue_t result = analyzerTask->checkForPackets(receiveArray.data(),
				receiveArray.size(), &packetFoundLen);
		if(result == HasReturnvaluesIF::RETURN_OK) {
			result = handlePacketReception(packetFoundLen);
			if(result != HasReturnvaluesIF::RETURN_OK) {
			    sif::debug << "RS485DeviceComIF::handleReceiveBuffer: Handling Buffer"
			            << " failed!" << std::endl;
				return result;
			}
		}
		else if(result == RingBufferAnalyzer::POSSIBLE_PACKET_LOSS) {
			// trigger event?
		    sif::debug << "RS485DeviceComIF::handleReceiveBuffer: Possible data loss"
		            << std::endl;
			continue;
		}
		else if(result == RingBufferAnalyzer::NO_PACKET_FOUND) {
			return HasReturnvaluesIF::RETURN_OK;
		}
	}

	return HasReturnvaluesIF::RETURN_OK;
}

// Just an echo function for testing
ReturnValue_t RS485DeviceComIF::handlePacketReception(size_t foundLen) {
	store_address_t storeId;
	RS485Cookie* memoryLeakCookie = new RS485Cookie(COM_FPGA);

	ReturnValue_t result = sendMessage(memoryLeakCookie,
			reinterpret_cast< unsigned char *>(const_cast<char*>("SendMessage")), 6);
	return result;
}

void RS485DeviceComIF::genericUartCallback(SystemContext context,
        xSemaphoreHandle sem) {
    BaseType_t higherPriorityTaskAwoken = pdFALSE;
    if(context == SystemContext::task_context) {
        BinarySemaphore::release(sem);
    }
    else {
        BinarySemaphore::releaseFromISR(sem,
                &higherPriorityTaskAwoken);
    }
    if(context == SystemContext::isr_context and
            higherPriorityTaskAwoken == pdPASS) {
        // Request a context switch before exiting ISR, as recommended
        // by FreeRTOS.
        TaskManagement::requestContextSwitch(CallContext::ISR);
    }
}


