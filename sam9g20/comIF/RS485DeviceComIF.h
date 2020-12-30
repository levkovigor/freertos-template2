#ifndef SAM9G20_COMIF_RS485DEVICECOMIF_H_
#define SAM9G20_COMIF_RS485DEVICECOMIF_H_

#include <fsfw/devicehandlers/DeviceCommunicationIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/osal/FreeRTOS/BinarySemaphore.h>
#include <fsfw/osal/FreeRTOS/TaskManagement.h>
#include <fsfwconfig/OBSWConfig.h>


extern "C" {

#include <hal/Drivers/UART.h>
}

enum RS485Steps: uint8_t {
    FPGA_1_ACTIVE,
	FPGA_2_ACTIVE,
    PCDU_VORAGO_ACTIVE,
    PL_VORAGO_ACTIVE,
    PL_PIC24_ACTIVE
};

class RS485DeviceComIF: public DeviceCommunicationIF,
public SystemObject,
public ExecutableObjectIF{
public:

	RS485DeviceComIF(object_id_t objectId);
	virtual ~RS485DeviceComIF();


 /** ExecutableObjectIF overrides */
	ReturnValue_t performOperation(uint8_t opCode) override;
	ReturnValue_t initialize() override;

 /** ExecutableObjectIF overrides */
    virtual ReturnValue_t initializeInterface(CookieIF * cookie) override;
    virtual ReturnValue_t sendMessage(CookieIF *cookie,
            const uint8_t * sendData, size_t sendLen) override;
    virtual ReturnValue_t getSendSuccess(CookieIF *cookie) override;
    virtual ReturnValue_t requestReceiveMessage(CookieIF *cookie,
            size_t requestLen) override;
    virtual ReturnValue_t readReceivedMessage(CookieIF *cookie,
            uint8_t **buffer, size_t *size) override;

    static constexpr uint8_t INTERFACE_ID = CLASS_ID::RS485_COM_IF;

    static constexpr ReturnValue_t RS485_INACTIVE = MAKE_RETURN_CODE(0x00);


private:

    uint8_t retryCount = 0;
    ReturnValue_t checkDriverState(uint8_t* retryCount);
    UARTgenericTransfer uartTransferFPGA1;
    BinarySemaphore uartSemaphoreFPGA1;
    UARTgenericTransfer uartTransferPCDU;
    BinarySemaphore uartSemaphorePCDU;

    static void genericUartCallback(SystemContext context,
            xSemaphoreHandle sem);

};

#endif /* BSP_COMIF_RS485DEVICECOMIF_H_ */
