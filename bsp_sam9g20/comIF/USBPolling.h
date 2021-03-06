#ifndef SAM9G20_COMIF_USBPOLLING_H_
#define SAM9G20_COMIF_USBPOLLING_H_

#include <fsfw/container/SharedRingBuffer.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/osal/freertos/BinarySemaphore.h>
#include <fsfw/tasks/ExecutableObjectIF.h>
#include <OBSWConfig.h>
#include <array>

class USBPolling: public SystemObject, public ExecutableObjectIF {
public:
	USBPolling(object_id_t objectId, SharedRingBuffer* usbRingBuffer);

	ReturnValue_t performOperation(uint8_t opCode) override;
	ReturnValue_t initialize() override;
private:

	static constexpr size_t USB_FRAME_MAX_SIZE =
	    		config::USB_FRAME_SIZE;

	struct UsbStruct {
		USBPolling* taskPtr;
		uint8_t* receiveBuffer;
		size_t dataReceived;
		// Specify whether buffer is read for read call.
		volatile bool bufferReady;
		SemaphoreHandle_t semaphore;
		UsbStruct* otherUsbStruct;
	};

	static volatile bool dataComingInTooFast;

	BinarySemaphore usbSemaphore1;
	std::array<uint8_t, USB_FRAME_MAX_SIZE> usbBuffer1;
	UsbStruct usbStruct1;

	BinarySemaphore usbSemaphore2;
	std::array<uint8_t, USB_FRAME_MAX_SIZE> usbBuffer2;
	UsbStruct usbStruct2;


	SharedRingBuffer* usbRingBuffer;

	void writeTransfer1ToRingBuffer();
	void writeTransfer2ToRingBuffer();

	void handleOverrunSituation();
	static void UsbDataReceived(void* usbPollingTask, unsigned char status,
			unsigned int received, unsigned int remaining);
};


#endif /* SAM9G20_COMIF_USBPOLLING_H_ */
