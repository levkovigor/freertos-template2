#include <sam9g20/comIF/RS485DeviceComIF.h>
#include <fsfw/tasks/TaskFactory.h>
#include <sam9g20/comIF/cookies/RS485Cookie.h>

extern "C" {
#include <hal/Drivers/UART.h>
	}


RS485DeviceComIF::RS485DeviceComIF(object_id_t objectId):
		SystemObject(objectId) {

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

	switch(device) {
	    case(RS485Devices::FPGA_1): {
	    	uartSemaphoreFPGA1.acquire();
	    	uartTransferFPGA1.writeSize = sendLen;
	    	uartTransferFPGA1.writeData = sendData;
	    	uartSemaphoreFPGA1.release();
	        break;
	    }
	    case(RS485Devices::FPGA_2): {
	        break;
	    }
	    case(RS485Devices::PCDU_VORAGO): {
	        break;
	    }
	    case(RS485Devices::PL_VORAGO): {
	        break;
	    }
	    case(RS485Devices::PL_PIC24): {
	        break;
	    }
	    default: {
	        // should not happen
	    	sif::error << "Unknown RS485 device" << std::endl;
	        break;
	    }
	    }

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

    switch(step) {
    case(RS485Devices::FPGA_1): {
        // Activate transceiver via GPIO
//    	uartTransferFPGA1.writeData = ;
//		uartTransferFPGA1.writeSize = ;
    	sif::info << "Sending to FPGA 1" << std::endl;
    	uartSemaphoreFPGA1.acquire();
    	UART_queueTransfer(&uartTransferFPGA1);
    	// Aquire semaphore, write new message to send, release semaphore
        break;
    }
    case(RS485Devices::FPGA_2): {
        // Activate transceiver via GPIO
//    	uartTransferFPGA2.writeData = ;
//		uartTransferFPGA2.writeSize = ;
    	sif::info << "Sending to FPGA 2" << std::endl;
    	uartSemaphorePCDU.acquire();
    	UART_queueTransfer(&uartTransferPCDU);
    	// Aquire semaphore, write new message to send, release semaphore
        break;
    }
    case(RS485Devices::PCDU_VORAGO): {
        // Activate transceiver and notify RS485 polling task by releasing
        // a semaphore so it can start sending packets.
    	sif::info << "Sending to PCDU" << std::endl;
    	uartSemaphorePCDU.acquire();
    	UART_queueTransfer(&uartTransferPCDU);

        break;
    }
    case(RS485Devices::PL_VORAGO): {
        // Activate transceiver and notify RS485 polling task by releasing
        // a semaphore so it can start sending packets.
    	sif::info << "Sending to PL_VORAGO" << std::endl;
        break;
    }
    case(RS485Devices::PL_PIC24): {
        // Activate transceiver and notify RS485 polling task by releasing
        // a semaphore so it can start sending packets.
    	sif::info << "Sending to PL_PIC24" << std::endl;
        break;
    }
    default: {
        // should not happen
        break;
    }
    }

    // printout and event.
    if(retryCount > 0) {
#ifdef DEBUG
        sif::error << "RS485Controller::performOperation: RS485Controller"
                << " driver was busy for " << (uint16_t) retryCount
                << " attempts!" << std::endl;
#endif
        retryCount = 0;
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t RS485DeviceComIF::initialize() {
    uartTransferFPGA1.bus = bus2_uart;
	uartTransferFPGA1.callback = genericUartCallback;
	uartTransferFPGA1.direction = write_uartDir;
	uartTransferFPGA1.writeData = reinterpret_cast< unsigned char *>(const_cast<char*>("FPGA1I"));
	uartTransferFPGA1.writeSize = 6;
	uartTransferFPGA1.postTransferDelay = 0;
//	uartTransferFPGA1.result = &transfer1Status;
	uartTransferFPGA1.semaphore = uartSemaphoreFPGA1.getSemaphore();

	uartTransferPCDU.bus = bus2_uart;
	uartTransferPCDU.callback = genericUartCallback;
	uartTransferPCDU.direction = write_uartDir;
	uartTransferPCDU.writeData = reinterpret_cast< unsigned char *>(const_cast<char*>("PCDUI"));
	uartTransferPCDU.writeSize = 6;
	uartTransferPCDU.postTransferDelay = 0;
//	uartTransferPCDU.result = &transfer2Status;
	uartTransferPCDU.semaphore = uartSemaphorePCDU.getSemaphore();

    return HasReturnvaluesIF::RETURN_OK;
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


