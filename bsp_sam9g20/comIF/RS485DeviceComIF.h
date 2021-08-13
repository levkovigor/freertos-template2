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
#include <mission/utility/uslpDataLinkLayer/USLPTransferFrame.h>
#include <mission/utility/uslpDataLinkLayer/UslpDataLinkLayer.h>

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
    static constexpr uint8_t MAX_TM_FRAMES_SENT_PER_CYCLE = 6;
    static constexpr uint8_t RETRY_COUNTER = 10;
    //! Default mutex timeout in milliseconds
    static constexpr TickType_t RS485_STANDARD_MUTEX_TIMEOUT = 20;

    RS485DeviceComIF(object_id_t objectId, object_id_t UslpDataLinkLayerId,
            object_id_t tcDestination, object_id_t tmStoreId, object_id_t tcStoreId);
    virtual ~RS485DeviceComIF();

    /**
     * @brief   DeviceComIF override, stores pointer to each device cookie in an array
     * @details Pointer to each device needs to be accessible by performOperation
     * @returns -@c RETURN_OK if all goes well
     *          -@c RETURN_FAILED If the cookie pointer is a nullpointer
     */
    virtual ReturnValue_t initializeInterface(CookieIF *cookie) override;

    /**
     * @brief   ExecutableObjectIF override, performs init of the UslpDataLinkLayer
     * @details Initializes:
     *          One VC for each device
     *          One Device MAP and an optional TmTC map for each VC
     * @returns -@c RETURN_OK if all goes well
     *          -@c RETURN_FAILED It there is an error with getting the LinkLayer Object
     */
    ReturnValue_t initialize() override;

    /**
     * @brief   ExecutableObjectIF override, performs send Operation in correct timeslot
     * @details Is initialized as fixed timeslot task with timeslot for each device
     * 			Only one timeslot for FPGAs as only one is active at a time
     * @param opCode Determines to which device messages are sent, enum in RS485Cookie
     * @returns -@c RETURN_OK always as Com errors are stored in the device cookies
     */
    ReturnValue_t performOperation(uint8_t opCode) override;

    /**
     * @brief  DeviceComIF override, fills frames sendBuffer, sets transfer status to TRANSFER_INIT
     * @details Only DeviceHandlers use this to send message, as only one message is sent
     *			from these at a time, one frame buffer for each timeslot suffices
     * @returns -@c RETURN_OK if buffer is filled
     * 			-@c CCSDSReturnValueIF return values if an error occured
     */
    ReturnValue_t sendMessage(CookieIF *cookie, const uint8_t *sendData, size_t sendLen) override;
    /**
     * @brief  DeviceComIF override, retrieves sending status
     * @details Make sure to set the time between calling sendMessage and getSendSuccess to
     *          one com cycle
     * @returns -@c RETURN_OK if frame was sent successful
     *          -@c RETURN_FAILED If the frame was not sent yet
     *          -@c UART driver return values if an error in the driver occured
     */
    ReturnValue_t getSendSuccess(CookieIF *cookie) override;
    /**
     * @brief  DeviceComIF override, sets cookie receive status to TRANSFER_INIT and resets buffer
     * @details The cookie signals the data link layer to place a received message in the receive
     *          buffer.
     * @returns -@c RETURN_OK if alway
     */
    ReturnValue_t requestReceiveMessage(CookieIF *cookie, size_t requestLen) override;
    /**
     * @brief  DeviceComIF override, checks for message in buffer and resets receive status to IDLE
     * @details Make sure to set the time between calling requestReceiveMessage and
     *          readReceivedMessage to one com cycle
     *          one com cycle
     * @returns -@c RETURN_OK if there is a message in the buffer
     *          -@c RETURN_FAILED If no frame was received yet
     */
    ReturnValue_t readReceivedMessage(CookieIF *cookie, uint8_t **buffer, size_t *size) override;

    static constexpr uint8_t INTERFACE_ID = CLASS_ID::RS485_COM_IF;

    static constexpr ReturnValue_t RS485_INACTIVE = MAKE_RETURN_CODE(0x00);

private:

    uint8_t retryCount = 0;
    uint8_t packetSentCounter = 0;

    /* Stores the act device cookies for each timeslot */
    std::array<RS485Cookie*, RS485Timeslot::TIMESLOT_COUNT_RS485> deviceCookies;

    // Stores the inactive devices cookies, they are swapped in setActive should it be necessary
    std::array<RS485Cookie*, RS485Timeslot::TIMESLOT_COUNT_RS485> deviceCookiesInactive;

    //Frame buffer arrays with largest frames as size
    std::array<std::array<uint8_t, TMTC_FRAME_MAX_LEN>, RS485Timeslot::TIMESLOT_COUNT_RS485> sendBufferFrame;
    std::array<std::array<uint8_t, config::RS485_COM_FPGA_TFDZ_SIZE>,
            RS485Timeslot::TIMESLOT_COUNT_RS485> receiveBufferDevice;

    object_id_t uslpDataLinkLayerId = objects::NO_OBJECT;
    UslpDataLinkLayer *uslpDataLinkLayer = nullptr;

    // Used for setting up the MAPs for TmTc
    object_id_t tcDestination = objects::NO_OBJECT;
    object_id_t tmStoreId = objects::NO_OBJECT;
    object_id_t tcStoreId = objects::NO_OBJECT;


    /**
     * @brief   Performs UART Transfer of DeviceCommands
     * @details One Command in one frame is sent per cycle
     */
    void handleSend(RS485Timeslot device, RS485Cookie *rs485Cookie);
    /**
     * @brief   Performs UART Transfer of  TM
     * @details Calls RS485TmTcTarget fillFrameBuffer
     */
    void handleTmSend(RS485Timeslot device, RS485Cookie *rs485Cookie);

    /**
     * @brief Adds all VCs and MAPs to the data link layer.
     * @returns @c RETURN_OK if all goes well
     */
    ReturnValue_t setupDataLinkLayer(UslpDataLinkLayer *dataLinkLayer);
    /**
     * @brief Adds a VC from a device cookie to the Data Link Layer
     * @returns @c RETURN_OK if all goes well
     */
    ReturnValue_t setupDeviceVC(UslpDataLinkLayer *dataLinkLayer, RS485Cookie *rs485Cookie);
    /**
     * @brief Checks if the current cookie is inactive and the other active, if true replaces it
     * @details Make sure to set the inactive device to active, otherwise this function will not
     *          switch the cookies.
     * @returns @c RETURN_OK if we have a valid cookie
     *          @c RETUNR FAILED if there is no possibility to get a non nullpointer cookie
     */
    ReturnValue_t setActive(RS485Timeslot timeslot);

    ReturnValue_t checkDriverState(uint8_t *retryCount);

};

#endif /* BSP_COMIF_RS485DEVICECOMIF_H_ */
