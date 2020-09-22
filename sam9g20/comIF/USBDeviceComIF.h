#ifndef SAM9G20_COMIF_USBDEVICECOMIF_H_
#define SAM9G20_COMIF_USBDEVICECOMIF_H_

#include <fsfw/container/SharedRingBuffer.h>
#include <fsfw/container/SimpleRingBuffer.h>
#include <fsfw/devicehandlers/DeviceCommunicationIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <config/tmtc/tmtcSize.h>
#include <array>


class USBDeviceComIF: public SystemObject, public DeviceCommunicationIF {
public:
	USBDeviceComIF(object_id_t objectId, SharedRingBuffer* usbRingBuffer);

	/** DeviceCommunicationIF implementation */
	ReturnValue_t initializeInterface(CookieIF * cookie) override;
	ReturnValue_t sendMessage(CookieIF *cookie, const uint8_t * sendData,
			size_t sendLen) override;
	ReturnValue_t getSendSuccess(CookieIF *cookie) override;
	ReturnValue_t requestReceiveMessage(CookieIF *cookie,
			size_t requestLen) override;
	virtual ReturnValue_t readReceivedMessage(CookieIF *cookie,
			uint8_t **buffer, size_t *size) override;
private:
	static constexpr uint16_t USB_FRAME_MAX_SIZE =
	    		tmtcsize::MAX_USB_FRAME_SIZE;
	std::array<uint8_t, USB_FRAME_MAX_SIZE> usbBuffer;
	SharedRingBuffer* usbRingBuffer = nullptr;
	//------------------------------------------------------------------------------
	/// Callback invoked when data has been received on the USB.
	//------------------------------------------------------------------------------
	static void UsbDataReceived(void* unused, unsigned char status,
			unsigned int received, unsigned int remaining);
	static void UsbDataSent(void* unused, unsigned char status,
			unsigned int received, unsigned int remaining);

	enum TransferStates {
		WRITE_SUCCESS,
		WRITE_BUSY,
		WRITE_FAILURE,
	};

	size_t sizeRead = 0;

	static size_t sentDataLen;
	static uint8_t errorStatus;

	TransferStates writeState = TransferStates::WRITE_SUCCESS;
	bool errorOccuredOnce = false;
};



#endif /* SAM9G20_COMIF_USBDEVICECOMIF_H_ */
