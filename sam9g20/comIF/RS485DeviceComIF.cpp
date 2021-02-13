/**
 * @file	RS485DeviceComIF.h
 * @brief	This file defines the RS485DeviceComIf class.
 * @date	22.12.2020
 * @author	L.Rajer
 */
#include <sam9g20/comIF/RS485DeviceComIF.h>
#include <fsfw/tasks/TaskFactory.h>
#include <sam9g20/comIF/cookies/RS485Cookie.h>
#include <mission/utility/uslpDataLinkLayer/USLPTransferFrame.h>
#include <mission/utility/uslpDataLinkLayer/UslpDataLinkLayer.h>
#include <mission/utility/uslpDataLinkLayer/UslpVirtualChannelIF.h>
#include <mission/utility/uslpDataLinkLayer/UslpVirtualChannel.h>
#include <mission/utility/uslpDataLinkLayer/UslpMapIF.h>
#include <mission/utility/uslpDataLinkLayer/UslpMapTmTc.h>
#include <mission/utility/uslpDataLinkLayer/UslpMapDevice.h>
#include <sam9g20/comIF/RS485BufferAnalyzerTask.h>
#include "GpioDeviceComIF.h"

extern "C" {
#include <hal/Drivers/UART.h>
}

RS485DeviceComIF::RS485DeviceComIF(object_id_t objectId, object_id_t UslpDataLinkLayerId,
        object_id_t tcDestination, object_id_t tmStoreId, object_id_t tcStoreId) :
        SystemObject(objectId), tcDestination(tcDestination), tmStoreId(tmStoreId), tcStoreId(
                tcStoreId), uslpDataLinkLayerId(UslpDataLinkLayerId) {
    // Necessary so we catch timeslots with no cookies
    for (int i = 0; i < RS485Timeslot::TIMESLOT_COUNT_RS485; i++) {
        deviceCookies[i] = nullptr;
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
    // TODO: Loop for devices, not timeslots
    for (int device = 0; device < RS485Timeslot::TIMESLOT_COUNT_RS485 - 3; device++) {
        RS485Cookie *rs485Cookie = deviceCookies[device];

        // VC Channel for device
        UslpVirtualChannelIF *virtualChannel;
        virtualChannel = new UslpVirtualChannel(rs485Cookie->getVcId(), rs485Cookie->getTfdzSize());

        // Add MAP for normal device communication
        UslpMapIF *mapDeviceCom;
        mapDeviceCom = new UslpMapDevice(rs485Cookie->getDevicComMapId(),
                receiveBufferDevice[device].data(), receiveBufferDevice[device].size());
        virtualChannel->addMapChannel(rs485Cookie->getDevicComMapId(), mapDeviceCom);

        // Add MAP for Tm and Tc
        if (rs485Cookie->getHasTmTc()) {
            UslpMapIF *mapTmTc;
            mapTmTc = new UslpMapTmTc(objects::USLP_MAPP_SERVICE, rs485Cookie->getTmTcMapId(),
                    tcDestination, tmStoreId, tcStoreId);
            virtualChannel->addMapChannel(rs485Cookie->getTmTcMapId(), mapTmTc);

            uslpDataLinkLayer->addVirtualChannel(rs485Cookie->getVcId(), virtualChannel);
        }
    }
    return HasReturnvaluesIF::RETURN_OK;
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

            // Normal device command to CCSDS board still need to be sent
            handleSend(timeslot, rs485Cookie);
            handleTmSend(timeslot, rs485Cookie);
            break;
        }
        case (RS485Timeslot::PCDU_VORAGO): {
            GpioDeviceComIF::enableTransceiverPCDU();
            handleSend(timeslot, rs485Cookie);
            break;
        }
        case (RS485Timeslot::PL_VORAGO): {
            handleSend(timeslot, rs485Cookie);
            GpioDeviceComIF::enableTransceiverVorago();
            break;
        }
        case (RS485Timeslot::PL_PIC24): {
            handleSend(timeslot, rs485Cookie);
            GpioDeviceComIF::enableTransceiverPIC24();
            break;
        }
        default: {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            // should not happen
            // TODO: Is there anything special to do in this case
            sif::error << "RS485 Device Number out of bounds" << std::endl;
#endif
            break;
        }
        }

    } else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "RS485 Device Cookies not initialized yet" << std::endl;
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
    RS485Timeslot timeslot = rs485Cookie->getTimeslot();

    // TODO: Mutex
    // Copy Message into corresponding sendFrameBuffer
    ReturnValue_t result = uslpDataLinkLayer->packFrame(sendData, sendLen,
            sendBufferFrame[timeslot].data(),
            rs485Cookie->getTfdzSize() + USLPTransferFrame::FRAME_OVERHEAD, rs485Cookie->getVcId(),
            rs485Cookie->getDevicComMapId());

    return result;

}

ReturnValue_t RS485DeviceComIF::getSendSuccess(CookieIF *cookie) {

    RS485Cookie *rs485Cookie = dynamic_cast<RS485Cookie*>(cookie);
    if (rs485Cookie->getComStatus() == ComStatusRS485::TRANSFER_SUCCESS) {
        rs485Cookie->setComStatus(ComStatusRS485::IDLE);
        return HasReturnvaluesIF::RETURN_OK;
    } else {
        // TODO: Generate event corresponding to error code stored in Cookie here
        rs485Cookie->setComStatus(ComStatusRS485::IDLE);
        return HasReturnvaluesIF::RETURN_FAILED;
    }

}

ReturnValue_t RS485DeviceComIF::requestReceiveMessage(CookieIF *cookie, size_t requestLen) {
    RS485Cookie *rs485Cookie = dynamic_cast<RS485Cookie*>(cookie);
    //TODO: set com status
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t RS485DeviceComIF::readReceivedMessage(CookieIF *cookie, uint8_t **buffer,
        size_t *size) {
    // TODO: Check for available messages
    RS485Cookie *rs485Cookie = dynamic_cast<RS485Cookie*>(cookie);
    *buffer = receiveBufferDevice[rs485Cookie->getTimeslot()].data();
    *size = rs485Cookie->getTfdzSize();
    return HasReturnvaluesIF::RETURN_OK;
}

void RS485DeviceComIF::handleSend(RS485Timeslot device, RS485Cookie *rs485Cookie) {
    int retval = 0;

    // Check if message is available
    if (rs485Cookie->getComStatus() == ComStatusRS485::TRANSFER_INIT_SUCCESS) {
        // Buffer is already filled, so just send it
        retval = UART_write(bus2_uart, sendBufferFrame[device].data(),
                rs485Cookie->getTfdzSize() + USLPTransferFrame::FRAME_OVERHEAD);
    }

    sendBufferFrame[device].fill(0);

    //TODO: Mutex for ComStatus
    rs485Cookie->setReturnValue(retval);
    if (retval != 0) {
        rs485Cookie->setComStatus(ComStatusRS485::FAULTY);
    } else {
        rs485Cookie->setComStatus(ComStatusRS485::TRANSFER_SUCCESS);
    }

}

void RS485DeviceComIF::handleTmSend(RS485Timeslot device, RS485Cookie *rs485Cookie) {

    // TODO: Check if downlink available
    for (packetSentCounter = 0;
    // We dont need to supply an input, as the packets come directly from the TmQueue
            uslpDataLinkLayer->packFrame(nullptr, 0, sendBufferFrame[device].data(),
                    rs485Cookie->getTfdzSize(), rs485Cookie->getVcId(), rs485Cookie->getTmTcMapId())
                    == HasReturnvaluesIF::RETURN_OK; packetSentCounter++) {

        // TODO: do something with result
        UART_write(bus2_uart, sendBufferFrame[device].data(),
                rs485Cookie->getTfdzSize() + USLPTransferFrame::FRAME_OVERHEAD);

        // Reset the buffer to all 0
        sendBufferFrame[device].fill(0);

        if (packetSentCounter >= MAX_TM_FRAMES_SENT_PER_CYCLE - 1) {
            break;
        }
    }
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

