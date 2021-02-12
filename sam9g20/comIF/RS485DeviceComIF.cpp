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

RS485DeviceComIF::RS485DeviceComIF(object_id_t objectId, object_id_t bufferAnalyzerId,
        object_id_t UslpDataLinkLayerId, object_id_t tcDestination, object_id_t tmStoreId,
        object_id_t tcStoreId) :
        SystemObject(objectId), bufferAnalyzerId(bufferAnalyzerId), tcDestination(tcDestination), tmStoreId(
                tmStoreId), tcStoreId(tcStoreId), uslpDataLinkLayerId(UslpDataLinkLayerId) {
    // Necessary so we catch timeslots with no cookies
    for (int i = 0; i < RS485Timeslot::TIMESLOT_COUNT_RS485; i++) {
        deviceCookies[i] = nullptr;
    }
}
RS485DeviceComIF::~RS485DeviceComIF() {
}

ReturnValue_t RS485DeviceComIF::initialize() {

    bufferAnalyzer = objectManager->get<RS485BufferAnalyzerTask>(bufferAnalyzerId);
    if (bufferAnalyzer == nullptr) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    uslpDataLinkLayer = objectManager->get<UslpDataLinkLayer>(uslpDataLinkLayerId);
    if (uslpDataLinkLayer == nullptr) {
        return HasReturnvaluesIF::RETURN_FAILED;
    } else {
        // TODO: Loop for devices, not timeslots
        for (int device = 0; device < RS485Timeslot::TIMESLOT_COUNT_RS485 - 3; device++) {
            RS485Cookie *rs485Cookie = dynamic_cast<RS485Cookie*>(deviceCookies[device]);

            UslpVirtualChannelIF *virtualChannel;
            virtualChannel = new UslpVirtualChannel(rs485Cookie->getVcId(),
                    rs485Cookie->getTfdzSize());

            // Add Map for normal device communication
            UslpMapIF *mapDeviceCom;
            mapDeviceCom = new UslpMapDevice(rs485Cookie->getDevicComMapId(),
                    receiveBufferDevice[device].data(), receiveBufferDevice[device].size());
            virtualChannel->addMapChannel(rs485Cookie->getDevicComMapId(), mapDeviceCom);

            // Add Map for Tm and Tc
            if (rs485Cookie->getHasTmTc()) {
                UslpMapIF *mapTmTc;
                mapTmTc = new UslpMapTmTc(objects::USLP_MAPP_SERVICE, rs485Cookie->getTmTcMapId(),
                        tcDestination, tmStoreId, tcStoreId);
                virtualChannel->addMapChannel(rs485Cookie->getTmTcMapId(), mapTmTc);

            }

            uslpDataLinkLayer->addVirtualChannel(rs485Cookie->getVcId(), virtualChannel);
        }
    }
    ReturnValue_t result = bufferAnalyzer->setvirtualChannelFrameSizes(&virtualChannelFrameSizes);
    return result;
}

ReturnValue_t RS485DeviceComIF::initializeInterface(CookieIF *cookie) {
    if (cookie != nullptr) {
        RS485Cookie *rs485Cookie = dynamic_cast<RS485Cookie*>(cookie);
        RS485Timeslot timeslot = rs485Cookie->getTimeslot();
        deviceCookies[timeslot] = cookie;
        //TODO: Returnvalue for map insertion
        virtualChannelFrameSizes.insert(
                std::pair<uint8_t, size_t>(rs485Cookie->getVcId(),
                        rs485Cookie->getTfdzSize() + USLPTransferFrame::FRAME_OVERHEAD));

        return HasReturnvaluesIF::RETURN_OK;
    } else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "RS485DeviceComIF::initializeInterface failed: Cookie is null pointer"
                << std::endl;
#endif
        return HasReturnvaluesIF::RETURN_FAILED;
    }

}

ReturnValue_t RS485DeviceComIF::performOperation(uint8_t opCode) {

    RS485Timeslot timeslot = static_cast<RS485Timeslot>(opCode);

    if (deviceCookies[timeslot] != nullptr) {
        // TODO: Make sure this does not turn into a bad cast as we have no exceptions
        RS485Cookie *rs485Cookie = dynamic_cast<RS485Cookie*>(deviceCookies[timeslot]);
        switch (timeslot) {
        case (RS485Timeslot::COM_FPGA): {
            // TODO: Check which FPGA is active (should probably be set via DeviceHandler in Cookie)
            GpioDeviceComIF::enableTransceiverFPGA1();
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

std::map<uint8_t, size_t>* RS485DeviceComIF::getVcidSizeMap() {
    return &virtualChannelFrameSizes;
}

void RS485DeviceComIF::handleSend(RS485Timeslot device, RS485Cookie *rs485Cookie) {
    int retval = 0;

    // Check if messag is available
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

