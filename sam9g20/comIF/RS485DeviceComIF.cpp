#include <sam9g20/comIF/RS485DeviceComIF.h>
#include <fsfw/tasks/TaskFactory.h>
#include <sam9g20/comIF/cookies/RS485Cookie.h>


extern "C" {
#include <hal/Drivers/UART.h>
	}


RS485DeviceComIF::RS485DeviceComIF(object_id_t objectId, object_id_t sharedRingBufferId):
		SystemObject(objectId),
		sharedRingBufferId(sharedRingBufferId){

}
RS485DeviceComIF::~RS485DeviceComIF() {
}



ReturnValue_t RS485DeviceComIF::initializeInterface(CookieIF *cookie) {

    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t RS485DeviceComIF::sendMessage(CookieIF *cookie,
        const uint8_t *sendData, size_t sendLen) {
	RS485Cookie * rs485Cookie = dynamic_cast<RS485Cookie *> (cookie);
		RS485Devices device = rs485Cookie->getDevice();

			sendArray[device].writeData = const_cast<uint8_t*>(sendData);
			sendArray[device].sendLen = sendLen;
			// UART drive error codes begin at -3, so -4 will be message available for us
			sendArray[device].status = -4;


    return HasReturnvaluesIF::RETURN_OK;
}



ReturnValue_t RS485DeviceComIF::getSendSuccess(CookieIF *cookie) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t RS485DeviceComIF::requestReceiveMessage(CookieIF *cookie,
        size_t requestLen) {
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t RS485DeviceComIF::readReceivedMessage(CookieIF *cookie,
        uint8_t **buffer, size_t *size) {
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
    if (sendArray[step].status == -4){
    	sendArray[step].status = UART_write(bus2_uart, sendArray[step].writeData, sendArray[step].sendLen);
    }
    // Reception
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


ReturnValue_t RS485DeviceComIF::initialize() {

//	uartTransferFPGA1.writeData = reinterpret_cast< unsigned char *>(const_cast<char*>("FPGA1I"));



	SharedRingBuffer* ringBuffer =
			objectManager->get<SharedRingBuffer>(sharedRingBufferId);
	if(ringBuffer == nullptr) {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	analyzerTask = new RingBufferAnalyzer(ringBuffer,
			AnalyzerModes::DLE_ENCODING);

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
	RS485Cookie* memoryLeakCookie = new RS485Cookie();
	memoryLeakCookie->setDevice(COM_FPGA);

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


