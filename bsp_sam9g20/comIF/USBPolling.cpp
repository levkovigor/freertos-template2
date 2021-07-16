#include "USBPolling.h"

#include <fsfw/osal/freertos/TaskManagement.h>
#include <fsfw/tasks/TaskFactory.h>
#include <fsfw/serviceinterface/ServiceInterface.h>

extern "C" {
#include <usb/device/cdc/CDCDSerialDriver.h>
}

volatile bool USBPolling::dataComingInTooFast = false;

USBPolling::USBPolling(object_id_t objectId, SharedRingBuffer* usbRingBuffer):
		SystemObject(objectId), usbRingBuffer(usbRingBuffer) {
	usbStruct1.taskPtr = this;
	usbStruct1.semaphore = usbSemaphore1.getSemaphore();
	usbStruct1.otherUsbStruct = &usbStruct2;
	usbStruct1.receiveBuffer = usbBuffer1.data();
	usbStruct1.bufferReady = true;

	usbStruct2.taskPtr = this;
	usbStruct2.semaphore = usbSemaphore2.getSemaphore();
	usbStruct2.otherUsbStruct = &usbStruct1;
	usbStruct2.receiveBuffer = usbBuffer2.data();
	usbStruct2.bufferReady = true;

}

ReturnValue_t USBPolling::initialize() {
	if(usbRingBuffer == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "USBPolling::initialize: Ring buffer is nullptr!" << std::endl;
#else
		sif::printError("USBPolling::initialize: Ring buffer is nullptr!\n");
#endif
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t USBPolling::performOperation(uint8_t opCode) {
	// Initiate transfer.
	ReturnValue_t result = usbSemaphore1.acquire();
	if(result != HasReturnvaluesIF::RETURN_OK) {
		// should not happen!
	}

	uint8_t sendResult = CDCDSerialDriver_Read(usbBuffer1.data(),
			usbBuffer1.size(),
			reinterpret_cast<TransferCallback>(UsbDataReceived),
			&usbStruct1);
	if(sendResult != USBD_STATUS_SUCCESS) {
		// This should not happen!
	}

	while(true) {
		usbSemaphore1.acquire(SemaphoreIF::TimeoutType::BLOCKING);
		writeTransfer1ToRingBuffer();

		usbSemaphore2.acquire(SemaphoreIF::TimeoutType::BLOCKING);
		writeTransfer2ToRingBuffer();

		if(dataComingInTooFast) {
			handleOverrunSituation();
		}
	}
	return HasReturnvaluesIF::RETURN_OK;
}


void USBPolling::writeTransfer1ToRingBuffer() {
	// write data in buffer 1 into ring buffer.
	usbRingBuffer->lockRingBufferMutex(MutexIF::TimeoutType::WAITING, 5);
	ReturnValue_t result = usbRingBuffer->writeData(usbBuffer1.data(),
			usbStruct1.dataReceived);
	if(result != HasReturnvaluesIF::RETURN_OK) {
		// trigger event or print debug output?
	}

	if(usbRingBuffer->getReceiveSizesFIFO()->full()) {
		// config error.
	}

	usbRingBuffer->getReceiveSizesFIFO()->insert(usbStruct1.dataReceived);
	usbStruct1.bufferReady = true;
	usbRingBuffer->unlockRingBufferMutex();
}

void USBPolling::writeTransfer2ToRingBuffer() {
	usbRingBuffer->lockRingBufferMutex(MutexIF::TimeoutType::WAITING, 5);
	ReturnValue_t result = usbRingBuffer->writeData(usbBuffer2.data(),
			usbStruct2.dataReceived);
	if(result != HasReturnvaluesIF::RETURN_OK) {
		// trigger event or print debug output?
	}

	if(usbRingBuffer->getReceiveSizesFIFO()->full()) {
		// config error.
	}

	usbRingBuffer->getReceiveSizesFIFO()->insert(usbStruct2.dataReceived);
	usbRingBuffer->unlockRingBufferMutex();

	usbStruct2.bufferReady = true;
}

void USBPolling::handleOverrunSituation() {
	// wait for a few cycles and reinitiate transfers?
	// or reboot after counting up a few cycles.
	// trigger event!

	TaskFactory::delayTask(500);
	int sendResult = 0;

	ReturnValue_t result = usbSemaphore1.acquire(
			SemaphoreIF::TimeoutType::BLOCKING);
	if(result == HasReturnvaluesIF::RETURN_OK) {
		// Reinitiate transfer.
		dataComingInTooFast = false;
		sendResult = CDCDSerialDriver_Read(usbBuffer1.data(),
				usbBuffer1.size(),
				reinterpret_cast<TransferCallback>(UsbDataReceived),
				&usbStruct1);
	}
	else {
		result = usbSemaphore2.acquire(
				SemaphoreIF::TimeoutType::BLOCKING);
		if(result == HasReturnvaluesIF::RETURN_OK) {
			dataComingInTooFast = false;
			// Reinitiate transfer.
			sendResult = CDCDSerialDriver_Read(usbBuffer2.data(),
					usbBuffer2.size(),
					reinterpret_cast<TransferCallback>(UsbDataReceived),
					&usbStruct2);
		}
		else {
			// trigger event (medium severity)
		}
	}

	if(sendResult != USBD_STATUS_SUCCESS) {
		// also not good, trigger event.
		return;
	}
}


void USBPolling::UsbDataReceived(void* usbPollingTask, unsigned char status,
		unsigned int received, unsigned int remaining)
{
	UsbStruct* usbStruct = static_cast<UsbStruct*>(usbPollingTask);
	BaseType_t higherPrioTaskWokenFromRelease;

	// release the semaphore in any case.
	ReturnValue_t result = BinarySemaphore::releaseFromISR(usbStruct->semaphore,
			&higherPrioTaskWokenFromRelease);
	usbStruct->dataReceived = received;
	if(result != HasReturnvaluesIF::RETURN_OK) {
		// config error
		return;
	}


	// Bus might be overloaded, don't intiate new transfer.
	if(dataComingInTooFast) {
		return;
	}

	// Initiate new transfer immediately.
	if(not usbStruct->otherUsbStruct->bufferReady) {
		// not good, data coming in too fast? we need to handle this
		// in main loop. It is a config error in any case
		dataComingInTooFast = true;
		return;
	}

	BaseType_t higherPrioTaskWokenFromTake;
	BaseType_t takeResult = xSemaphoreTakeFromISR(
			usbStruct->otherUsbStruct->semaphore,
			&higherPrioTaskWokenFromTake);
	if(takeResult != pdTRUE) {
		// config error
		return;
	}

	uint8_t sendResult = CDCDSerialDriver_Read(
			usbStruct->otherUsbStruct->receiveBuffer,
			USB_FRAME_MAX_SIZE,
			reinterpret_cast<TransferCallback>(UsbDataReceived),
			usbStruct->otherUsbStruct);

	if(sendResult != USBD_STATUS_SUCCESS) {
		// Should not happen!
	}

	if((higherPrioTaskWokenFromRelease == pdTRUE) or
			(higherPrioTaskWokenFromTake == pdTRUE)) {
		TaskManagement::requestContextSwitch(CallContext::ISR);
	}

    // Check that data has been received successfully
	// we should check whether bytes were discarded (config error)
	// and check the status of the transfer.
}

