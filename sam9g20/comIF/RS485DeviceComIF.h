/**
 * @file	RS485DeviceComIF.h
 * @brief	This file defines the RS485DeviceComIf class.
 * @date	22.12.2020
 * @author	L.Rajer
 */

#ifndef SAM9G20_COMIF_RS485DEVICECOMIF_H_
#define SAM9G20_COMIF_RS485DEVICECOMIF_H_

#include <fsfw/devicehandlers/DeviceCommunicationIF.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/tasks/ExecutableObjectIF.h>
#include <sam9g20/comIF/cookies/RS485Cookie.h>
#include <fsfw/osal/FreeRTOS/BinarySemaphore.h>
#include <fsfw/osal/FreeRTOS/TaskManagement.h>
#include <fsfwconfig/OBSWConfig.h>
#include <mission/utility/USLPTransferFrame.h>
#include <sam9g20/comIF/RS485TmTcTarget.h>

extern "C" {

#include <hal/Drivers/UART.h>
}

/**
 * @brief   Struct for communication between sendMessage and performOperation
 * @details In the future, this should be a specialized class like TMTCMessage
 * 			with a message queue for each device
 */

/**
 * @brief   This class is the sending and receiving interface for the RS485 Bus.
 * @details All communication works via the sendMessage, getSendSuccess, requestReceiveMessage
 * 			and readReceivedMessage functions. performOperation handles the actual sending as
 * 			this needs to be timed right with GPIOS.
 */
class RS485DeviceComIF: public DeviceCommunicationIF,
        public SystemObject,
        public ExecutableObjectIF {
public:

    static constexpr size_t TMTC_FRAME_MAX_LEN = config::RS485_MAX_SERIAL_FRAME_SIZE;
    static constexpr uint8_t MAX_TM_FRAMES_SENT_PER_CYCLE = 5;
    static constexpr uint8_t RETRY_COUNTER = 10;
    static constexpr char defaultMessage[] = { 'O', 'n', 'e', ' ', 'P', 'i', 'n', 'g', ' ', 'o',
            'n', 'l', 'y', ' ' };

    RS485DeviceComIF(object_id_t objectId, object_id_t tmTcTargetId);
    virtual ~RS485DeviceComIF();

    /**
     * @brief   ExecutableObjectIF override, performs send Operation in correct timeslot
     * @details Is initialized as fixed timeslot task with timeslot for each device
     * 			Only one timeslot for FPGAs as only one is active at a time
     * @param opCode Determines to which device messages are sent, enum in RS485Cookie
     * @returns -@c RETURN_OK always as Com errors are stored in the device cookies
     */
    ReturnValue_t performOperation(uint8_t opCode) override;
    /**
     * @brief   ExecutableObjectIF override, performs init of various buffers and queues
     * @details Initializes:
     * 			One Buffers for each device
     * 			Shared Ring Buffer analyzer
     * @param opCode Determines to which device messages are sent, enum in RS485Cookie
     * @returns -@c RETURN_OK if ring buffer analyzer init goes well
     * 			-@c RETURN_FALIED if ring buffer analyzer init fails
     */
    ReturnValue_t initialize() override;

    /**
     * @brief   DeviceComIF override, stores pointer to each device cookie
     * @details Pointer to each device needs to be accessible by performOperation
     */
    virtual ReturnValue_t initializeInterface(CookieIF *cookie) override;
    /**
     * @brief  DeviceComIF override, fills frames sendBuffer
     * @details Only DeviceHandlers use this to send message, as only one message is sent
     *			from these at a time, one frame buffer for each device suffices
     * @returns -@c RETURN_OK if buffer is filled
     * 			-@c RETURN_FALIED if there already is a message pending
     */
    virtual ReturnValue_t sendMessage(CookieIF *cookie, const uint8_t *sendData, size_t sendLen)
            override;
    virtual ReturnValue_t getSendSuccess(CookieIF *cookie) override;
    virtual ReturnValue_t requestReceiveMessage(CookieIF *cookie, size_t requestLen) override;
    virtual ReturnValue_t readReceivedMessage(CookieIF *cookie, uint8_t **buffer, size_t *size)
            override;

    static constexpr uint8_t INTERFACE_ID = CLASS_ID::RS485_COM_IF;

    static constexpr ReturnValue_t RS485_INACTIVE = MAKE_RETURN_CODE(0x00);

private:

    uint8_t retryCount = 0;
    uint8_t packetSentCounter = 0;

    // Stores one cookie for each device to communicate between ExecutableObjectIF overrides and DeviceComIF overrides
    std::array<CookieIF*, RS485Devices::DEVICE_COUNT_RS485> deviceCookies;

    // Frame buffers for each device
    std::array<uint8_t, config::RS485_COM_FPGA_TFDZ_SIZE + USLPTransferFrame::FRAME_OVERHEAD> transmitBufferFPGA;
    std::array<uint8_t, config::RS485_PCDU_VORAGO_TFDZ_SIZE + USLPTransferFrame::FRAME_OVERHEAD> transmitBufferPCDU;
    std::array<uint8_t, config::RS485_PAYLOAD_VORAGO_TFDZ_SIZE + USLPTransferFrame::FRAME_OVERHEAD> transmitBufferVorago;
    std::array<uint8_t, config::RS485_PAYLOAD_PIC24_TFDZ_SIZE + USLPTransferFrame::FRAME_OVERHEAD> transmitBufferPIC24;

    //Array with pointers to frame buffers
    std::array<USLPTransferFrame*, RS485Devices::DEVICE_COUNT_RS485> sendBuffer;

    // Used for handling the TM Queue, this class is already big enough
    object_id_t tmTcTargetId = objects::NO_OBJECT;

    RS485TmTcTarget *tmTcTarget = nullptr;

    /**
     * @brief  Initializes one TransferFrame class with buffer for each device
     * @details Most of the values for a device like e.g. VCID, SCID, etc.
     * 			stay the same, so they are set here and not changed later
     */
    void initTransferFrameSendBuffers();
    /**
     * @brief   Performs UART Transfer of DeviceCommands
     * @details One Command in one frame is sent per cycle
     */
    void handleSend(RS485Devices device, RS485Cookie *rs485Cookie);
    /**
     * @brief   Performs UART Transfer of  TM
     * @details Calls RS485TmTcTarget fillFrameBuffer
     */
    void handleTmSend(RS485Devices device, RS485Cookie *rs485Cookie);
    static void genericUartCallback(SystemContext context,
    xSemaphoreHandle sem);

    ReturnValue_t checkDriverState(uint8_t *retryCount);

};

#endif /* BSP_COMIF_RS485DEVICECOMIF_H_ */
