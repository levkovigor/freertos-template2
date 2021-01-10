#include <sam9g20/comIF/RS485DeviceComIF.h>
#include <fsfw/tasks/TaskFactory.h>
#include <sam9g20/comIF/cookies/RS485Cookie.h>
#include <mission/utility/USLPTransferFrame.h>

#include "GpioDeviceComIF.h"

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
	if (cookie != nullptr){
	RS485Cookie * rs485Cookie = dynamic_cast<RS485Cookie *> (cookie);
	RS485Devices device = rs485Cookie->getDevice();
	deviceCookies[device] = cookie;
    return HasReturnvaluesIF::RETURN_OK;
	}
	else{
		sif::error << "RS485DeviceComIF::initializeInterface failed: Cookie is null pointer" << std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}

}



ReturnValue_t RS485DeviceComIF::performOperation(uint8_t opCode) {

    RS485Devices device = static_cast<RS485Devices>(opCode);
    if (deviceCookies[device] != nullptr){
		RS485Cookie * rs485Cookie = dynamic_cast<RS485Cookie *> (deviceCookies[device]);

		switch(device) {
		case(RS485Devices::COM_FPGA): {
			// Activate transceiver via GPIO
			// Check which FPGA is active (should probably be set via DeviceHandler)
#ifdef DEBUG
			sif::info << "Sending to FPGA" << std::endl;
#endif
			GpioDeviceComIF::enableTransceiverFPGA1();
			break;
		}
		case(RS485Devices::PCDU_VORAGO): {
#ifdef DEBUG
			sif::info << "Sending to PCDU" << std::endl;
#endif
			GpioDeviceComIF::enableTransceiverPCDU();
			break;
		}
		case(RS485Devices::PL_VORAGO): {
#ifdef DEBUG
			sif::info << "Sending to PL_VORAGO" << std::endl;
#endif
			GpioDeviceComIF::enableTransceiverVorago();
			break;
		}
		case(RS485Devices::PL_PIC24): {
#ifdef DEBUG
			sif::info << "Sending to PL_PIC24" << std::endl;
#endif
			GpioDeviceComIF::enableTransceiverPIC24();
			break;
		}
		default: {
#ifdef DEBUG
			// should not happen
			sif::error << "RS485 Device Number out of bounds" << std::endl;
#endif
			break;
		}
		}
		handleSend(device, rs485Cookie);
    }
    else {
    	sif::error << "RS485 Device Cookies not initialized yet" << std::endl;
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
	if (rs485Cookie->getComStatus() == ComStatusRS485::IDLE){
		sendQueue[device].sendLen = sendLen;
		sendQueue[device].writeData = const_cast<uint8_t*>(sendData);
		rs485Cookie->setComStatus(ComStatusRS485::TRANSFER_INIT_SUCCESS);
		// Resets return value from last transfer
		rs485Cookie->setReturnValue(0);

    	return HasReturnvaluesIF::RETURN_OK;
	}
	else {
		sif::error << "RS485DeviceComIF::sendMessage: Device queue full" << std::endl;
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

void RS485DeviceComIF::handleSend(RS485Devices device, RS485Cookie* rs485Cookie){

	(void) std::memcpy(transferFrameFPGA->getDataZone(), sendQueue[device].writeData, sendQueue[device].sendLen);

	int retval = UART_write(bus2_uart, transmitBufferFPGA.data(), transmitBufferFPGA.size());

	rs485Cookie->setReturnValue(retval);
	if(retval != 0){
		rs485Cookie->setComStatus(ComStatusRS485::FAULTY);
	}
	else{
		rs485Cookie->setComStatus(ComStatusRS485::TRANSFER_SUCCESS);
	}

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

ReturnValue_t RS485DeviceComIF::checkDriverState(uint8_t* retryCount) {
    UARTdriverState readState = UART_getDriverState(bus2_uart,
            read_uartDir);
    UARTdriverState writeState = UART_getDriverState(bus2_uart,
            write_uartDir);
    if(readState != 0x00 or writeState != 0x00) {
        if(readState == 0x33 or writeState == 0x33) {
            // erroneous state!
#ifdef DEBUG
            sif::error << "RS485Controller::performOperation: RS485 driver"
                    " in invalid state!" << std::endl;
#endif
        }
        // config error, wait 1 ms and try again up to 10 times.
        for(uint8_t idx = 0; idx < RETRY_COUNTER; idx++) {
            TaskFactory::delayTask(1);
            readState = UART_getDriverState(bus2_uart,
                    read_uartDir);
            writeState = UART_getDriverState(bus2_uart,
                    write_uartDir);
            if(readState == 0x00 and writeState == 0x00) {
                return HasReturnvaluesIF::RETURN_OK;
            }
        }
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    return HasReturnvaluesIF::RETURN_OK;
}


