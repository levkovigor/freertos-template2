/**
 * @file	RS485DeviceComIF.h
 * @brief	This file defines the RS485DeviceComIf class.
 * @date	22.12.2020
 * @author	L.Rajer
 */
#include <sam9g20/comIF/RS485DeviceComIF.h>
#include <fsfw/tasks/TaskFactory.h>
#include <sam9g20/comIF/cookies/RS485Cookie.h>
#include <fsfw/ipc/MutexHelper.h>
#include <fsfw/osal/FreeRTOS/Mutex.h>
#include <mission/utility/uslpDataLinkLayer/USLPTransferFrame.h>
#include <mission/utility/uslpDataLinkLayer/UslpDataLinkLayer.h>
#include <mission/utility/uslpDataLinkLayer/UslpVirtualChannelIF.h>
#include <mission/utility/uslpDataLinkLayer/UslpVirtualChannel.h>
#include <mission/utility/uslpDataLinkLayer/UslpMapIF.h>
#include <mission/utility/uslpDataLinkLayer/UslpMapTmTc.h>
#include <mission/utility/uslpDataLinkLayer/UslpMapDevice.h>
#include <sam9g20/comIF/RS485BufferAnalyzerTask.h>
#include <AT91SAM9G20.h>
#include "GpioDeviceComIF.h"

extern "C" {
#include <hal/Drivers/UART.h>
}

RS485DeviceComIF::RS485DeviceComIF(object_id_t objectId, object_id_t UslpDataLinkLayerId,
        object_id_t tcDestination, object_id_t tmStoreId, object_id_t tcStoreId) :
        SystemObject(objectId), uslpDataLinkLayerId(UslpDataLinkLayerId), tcDestination(
                tcDestination), tmStoreId(tmStoreId), tcStoreId(tcStoreId) {
    // Necessary so we catch timeslots with no cookies
    for (int i = 0; i < RS485Timeslot::TIMESLOT_COUNT_RS485; i++) {
        deviceCookies[i] = nullptr;
        deviceCookiesInactive[i] = nullptr;
    }
}
RS485DeviceComIF::~RS485DeviceComIF() {
}

ReturnValue_t RS485DeviceComIF::initializeInterface(CookieIF *cookie) {
    if (cookie != nullptr) {
        RS485Cookie *rs485Cookie = dynamic_cast<RS485Cookie*>(cookie);
        RS485Timeslot timeslot = rs485Cookie->getTimeslot();
        if (rs485Cookie->getIsActive()) {
            deviceCookies[timeslot] = rs485Cookie;
        } else {
            deviceCookiesInactive[timeslot] = rs485Cookie;
        }
        return HasReturnvaluesIF::RETURN_OK;
    } else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "RS485DeviceComIF::initializeInterface failed: Cookie is null pointer"
                << std::endl;
#endif
        return HasReturnvaluesIF::RETURN_FAILED;
    }

}

ReturnValue_t RS485DeviceComIF::initialize() {

    uslpDataLinkLayer = objectManager->get<UslpDataLinkLayer>(uslpDataLinkLayerId);
    if (uslpDataLinkLayer == nullptr) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    return setupDataLinkLayer(uslpDataLinkLayer);
}

ReturnValue_t RS485DeviceComIF::performOperation(uint8_t opCode) {

    RS485Timeslot timeslot = static_cast<RS485Timeslot>(opCode);

// Set current active device, also checks for nullpointers
    if (setActive(timeslot) == HasReturnvaluesIF::RETURN_OK) {
        RS485Cookie *rs485Cookie = deviceCookies[timeslot];
        switch (timeslot) {
        case (RS485Timeslot::COM_FPGA): {
            // Check which transceiver to enable
            if (rs485Cookie->getVcId() == config::RS485_USLP_VCID_COM_FPGA_1) {
                GpioDeviceComIF::enableTransceiverFPGA1();
            } else {
                GpioDeviceComIF::enableTransceiverFPGA2();
            }
            // Set Baudrate
            AT91C_BASE_US2->US_BRGR = int(BOARD_MCK / (16 * rs485Cookie->getBaudrate()));

            // Normal device command to CCSDS board still need to be sent
            handleSend(timeslot, rs485Cookie);
            handleTmSend(timeslot, rs485Cookie);
            break;
        }
        case (RS485Timeslot::PCDU_VORAGO): {
            GpioDeviceComIF::enableTransceiverPCDU();
            // Set Baudrate
            AT91C_BASE_US2->US_BRGR = int(BOARD_MCK / (16 * rs485Cookie->getBaudrate()));
            handleSend(timeslot, rs485Cookie);
            break;
        }
        case (RS485Timeslot::PL_VORAGO): {
            GpioDeviceComIF::enableTransceiverVorago();
            // Set Baudrate
            AT91C_BASE_US2->US_BRGR = int(BOARD_MCK / (16 * rs485Cookie->getBaudrate()));
            handleSend(timeslot, rs485Cookie);
            break;
        }
        case (RS485Timeslot::PL_PIC24): {
            GpioDeviceComIF::enableTransceiverPIC24();
            // Set Baudrate
            AT91C_BASE_US2->US_BRGR = int(BOARD_MCK / (16 * rs485Cookie->getBaudrate()));
            handleSend(timeslot, rs485Cookie);
            break;
        }
        default: {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            // should never happen
            sif::error << "RS485 Device Number out of bounds" << std::endl;
#endif
            break;
        }
        }

    } else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "Not allRS485 Device Cookies not initialized yet" << std::endl;
#endif
    }

    if (retryCount > 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "RS485DeviceComIF::performOperation: RS485DeviceComIF"
                << " driver was busy for " << (uint16_t) retryCount << " attempts!" << std::endl;
#endif
        retryCount = 0;
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t RS485DeviceComIF::sendMessage(CookieIF *cookie, const uint8_t *sendData,
        size_t sendLen) {
    RS485Cookie *rs485Cookie = dynamic_cast<RS485Cookie*>(cookie);
    ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;
    RS485Timeslot timeslot = rs485Cookie->getTimeslot();
    MutexHelper mutexLock(rs485Cookie->getSendMutexHandle(), MutexIF::TimeoutType::WAITING,
            RS485_STANDARD_MUTEX_TIMEOUT);

    // Copy Message into corresponding sendFrameBuffer
    result = uslpDataLinkLayer->packFrame(sendData, sendLen, sendBufferFrame[timeslot].data(),
            sendBufferFrame[timeslot].size(), rs485Cookie->getVcId(),
            rs485Cookie->getDevicComMapId());
    if (result != HasReturnvaluesIF::RETURN_OK) {
        rs485Cookie->setComStatusSend(ComStatusRS485::FAULTY);
    } else {
        rs485Cookie->setComStatusSend(ComStatusRS485::TRANSFER_INIT);
    }
    return result;
}

ReturnValue_t RS485DeviceComIF::getSendSuccess(CookieIF *cookie) {
    RS485Cookie *rs485Cookie = dynamic_cast<RS485Cookie*>(cookie);
    ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;
    MutexHelper mutexLock(rs485Cookie->getSendMutexHandle(), MutexIF::TimeoutType::WAITING,
            RS485_STANDARD_MUTEX_TIMEOUT);
    // Successful sending has occured
    if (rs485Cookie->getComStatusSend() == ComStatusRS485::TRANSFER_SUCCESS) {
        result = HasReturnvaluesIF::RETURN_OK;
        /* If this happens, the time between sendMessage and getSendSuccess is not long enough
         or there is a problem with the send order */
    } else if (rs485Cookie->getComStatusSend() == ComStatusRS485::IDLE
            || rs485Cookie->getComStatusSend() == ComStatusRS485::TRANSFER_INIT) {
        result = HasReturnvaluesIF::RETURN_FAILED;
        return result;
        // Faulty transfer
    } else {
        result = rs485Cookie->getReturnValue();
    }
    rs485Cookie->setComStatusSend(ComStatusRS485::IDLE);
    return result;
}

ReturnValue_t RS485DeviceComIF::requestReceiveMessage(CookieIF *cookie, size_t requestLen) {
    RS485Cookie *rs485Cookie = dynamic_cast<RS485Cookie*>(cookie);
    // Com Status and receive Buffer is protected by mutex
    MutexHelper mutexLock(rs485Cookie->getReceiveMutexHandle(), MutexIF::TimeoutType::WAITING,
            RS485_STANDARD_MUTEX_TIMEOUT);
    // Reset the receive buffer
    receiveBufferDevice[rs485Cookie->getTimeslot()].fill(0);
    // This signals the DataLinkLayer that a message can be placed in the buffer
    rs485Cookie->setComStatusReceive(ComStatusRS485::TRANSFER_INIT);
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t RS485DeviceComIF::readReceivedMessage(CookieIF *cookie, uint8_t **buffer,
        size_t *size) {
    ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;
    RS485Cookie *rs485Cookie = dynamic_cast<RS485Cookie*>(cookie);
    // Com Status and receive Buffer is protected by mutex
    MutexHelper mutexLock(rs485Cookie->getReceiveMutexHandle(), MutexIF::TimeoutType::WAITING,
            RS485_STANDARD_MUTEX_TIMEOUT);
    if (rs485Cookie->getComStatusReceive() == ComStatusRS485::TRANSFER_SUCCESS) {
        *buffer = receiveBufferDevice[rs485Cookie->getTimeslot()].data();
        *size = rs485Cookie->getTfdzSize();
        rs485Cookie->setComStatusReceive(ComStatusRS485::IDLE);
        result = HasReturnvaluesIF::RETURN_OK;
        rs485Cookie->setComStatusReceive(ComStatusRS485::IDLE);
        // If a message has not been read yet, we do not change the status, only if there is a fault
    } else if (rs485Cookie->getComStatusReceive() != ComStatusRS485::TRANSFER_INIT) {
        rs485Cookie->setComStatusReceive(ComStatusRS485::IDLE);
    }
    return result;
}

void RS485DeviceComIF::handleSend(RS485Timeslot device, RS485Cookie *rs485Cookie) {
    int retval = 0;
    // Com Status, retval and send Buffer is protected by mutex
    MutexHelper mutexLock(rs485Cookie->getSendMutexHandle(), MutexIF::TimeoutType::WAITING,
            RS485_STANDARD_MUTEX_TIMEOUT);
    // Check if message is available
    if (rs485Cookie->getComStatusSend() == ComStatusRS485::TRANSFER_INIT) {
        // Buffer is already filled, so just send it
        retval = UART_write(bus2_uart, sendBufferFrame[device].data(),
                rs485Cookie->getTfdzSize() + USLPTransferFrame::FRAME_OVERHEAD);
        rs485Cookie->setReturnValue(retval);

        sendBufferFrame[device].fill(0);
        if (retval != 0) {
            rs485Cookie->setComStatusSend(ComStatusRS485::FAULTY);
        } else {
            rs485Cookie->setComStatusSend(ComStatusRS485::TRANSFER_SUCCESS);
        }
    }

}

void RS485DeviceComIF::handleTmSend(RS485Timeslot device, RS485Cookie *rs485Cookie) {
    int retval = 0;
// TODO: Check if downlink available
    // Send Buffer is protected by mutex
    MutexHelper mutexLock(rs485Cookie->getSendMutexHandle(), MutexIF::TimeoutType::WAITING,
            RS485_STANDARD_MUTEX_TIMEOUT);
    for (packetSentCounter = 0;
            // We dont need to supply an input, as the packets come directly from the TmQueue
            uslpDataLinkLayer->packFrame(nullptr, 0, sendBufferFrame[device].data(),
                    sendBufferFrame[device].size(), rs485Cookie->getVcId(),
                    rs485Cookie->getTmTcMapId()) == HasReturnvaluesIF::RETURN_OK;
            packetSentCounter++) {

        retval = UART_write(bus2_uart, sendBufferFrame[device].data(),
                rs485Cookie->getTfdzSize() + USLPTransferFrame::FRAME_OVERHEAD);
        // Faulty Tm Communicaton will get caught by get Send Success, even if the previous device
        // command message was successful (A fault here will likely lead to a fault there)
        if (retval != 0) {
            rs485Cookie->setComStatusSend(ComStatusRS485::FAULTY);
            rs485Cookie->setReturnValue(retval);
        }
        // Reset the buffer to all 0
        sendBufferFrame[device].fill(0);

        if (packetSentCounter >= MAX_TM_FRAMES_SENT_PER_CYCLE - 1) {
            break;
        }
    }
}

ReturnValue_t RS485DeviceComIF::setupDataLinkLayer(UslpDataLinkLayer *dataLinkLayer) {
    ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;
    for (int timeslot = 0; timeslot < RS485Timeslot::TIMESLOT_COUNT_RS485; timeslot++) {
        // Handle active devices
        if (deviceCookies[timeslot] != nullptr) {
            result = setupDeviceVC(dataLinkLayer, deviceCookies[timeslot]);
        }
        // Handle redundant inactive devices
        if (deviceCookiesInactive[timeslot] != nullptr) {
            result = setupDeviceVC(dataLinkLayer, deviceCookiesInactive[timeslot]);
        }

    }

    return result;
}

ReturnValue_t RS485DeviceComIF::setupDeviceVC(UslpDataLinkLayer *dataLinkLayer,
        RS485Cookie *rs485Cookie) {
    // VC Channel for device
    UslpVirtualChannelIF *virtualChannel;
    virtualChannel = new UslpVirtualChannel(rs485Cookie->getVcId(), rs485Cookie->getTfdzSize());

    // Add MAP for normal device communication
    UslpMapIF *mapDeviceCom;
    mapDeviceCom = new UslpMapDevice(rs485Cookie->getDevicComMapId(),
            receiveBufferDevice[rs485Cookie->getTimeslot()].data(),
            receiveBufferDevice[rs485Cookie->getTimeslot()].size(), rs485Cookie);
    virtualChannel->addMapChannel(rs485Cookie->getDevicComMapId(), mapDeviceCom);

    // Add MAP for Tm and Tc
    if (rs485Cookie->getHasTmTc()) {
        UslpMapIF *mapTmTc;
        mapTmTc = new UslpMapTmTc(objects::USLP_MAPP_SERVICE, rs485Cookie->getTmTcMapId(),
                tcDestination, tmStoreId, tcStoreId);
        virtualChannel->addMapChannel(rs485Cookie->getTmTcMapId(), mapTmTc);
    }
    dataLinkLayer->addVirtualChannel(rs485Cookie->getVcId(), virtualChannel);
    // TODO : More checks
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t RS485DeviceComIF::setActive(RS485Timeslot timeslot) {
    ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;
    bool isValidActive = (deviceCookies[timeslot] != nullptr);
    bool isValidInactive = (deviceCookiesInactive[timeslot] != nullptr);

    if (isValidActive) {
        if (isValidInactive) {
            // Swap if both conditions match, otherwise leave it at its current state
            if (!deviceCookies[timeslot]->getIsActive()
                    && deviceCookiesInactive[timeslot]->getIsActive()) {
                RS485Cookie *swap = deviceCookies[timeslot];
                deviceCookies[timeslot] = deviceCookiesInactive[timeslot];
                deviceCookiesInactive[timeslot] = swap;
            }
        }
        result = HasReturnvaluesIF::RETURN_OK;
    } else {
        // Something went wrong with the active cookie, but luckily, we have a working inactive cookie
        // and can replace it
        if (isValidInactive) {
            deviceCookies[timeslot] = deviceCookiesInactive[timeslot];
            deviceCookiesInactive[timeslot] = nullptr;
            result = HasReturnvaluesIF::RETURN_OK;
            // Both cookies are nullpointers, so there is no hope
        } else {
            result = HasReturnvaluesIF::RETURN_FAILED;
        }
    }

    return result;
}

ReturnValue_t RS485DeviceComIF::checkDriverState(uint8_t *retryCount) {
    UARTdriverState readState = UART_getDriverState(bus2_uart, read_uartDir);
    UARTdriverState writeState = UART_getDriverState(bus2_uart, write_uartDir);
    if (readState != 0x00 or writeState != 0x00) {
        if (readState == 0x33 or writeState == 0x33) {
            // erroneous state!
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "RS485Controller::performOperation: RS485 driver"
                    " in invalid state!" << std::endl;
#endif
        }
        // config error, wait 1 ms and try again up to 10 times.
        for (uint8_t idx = 0; idx < RETRY_COUNTER; idx++) {
            TaskFactory::delayTask(1);
            readState = UART_getDriverState(bus2_uart, read_uartDir);
            writeState = UART_getDriverState(bus2_uart, write_uartDir);
            if (readState == 0x00 and writeState == 0x00) {
                return HasReturnvaluesIF::RETURN_OK;
            }
        }
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    return HasReturnvaluesIF::RETURN_OK;
}

