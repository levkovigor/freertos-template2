/**
 * @file	RS485DeviceComIF.h
 * @brief	This file defines the RS485DeviceComIf class.
 * @date	22.12.2020
 * @author	L.Rajer
 */
#include <sam9g20/comIF/RS485DeviceComIF.h>
#include <sam9g20/comIF/RS485TmTcTarget.h>
#include <fsfw/tasks/TaskFactory.h>
#include <sam9g20/comIF/cookies/RS485Cookie.h>
#include <mission/utility/USLPTransferFrame.h>

#include "GpioDeviceComIF.h"

extern "C" {
#include <hal/Drivers/UART.h>
}

RS485DeviceComIF::RS485DeviceComIF(object_id_t objectId, object_id_t tmTcTargetId) :
        SystemObject(objectId), tmTcTargetId(tmTcTargetId) {

    for (int i = 0; i < RS485Devices::DEVICE_COUNT_RS485; i++) {
        deviceCookies[i] = nullptr;
    }
}
RS485DeviceComIF::~RS485DeviceComIF() {
}

ReturnValue_t RS485DeviceComIF::initialize() {

    // Init of the default transfer frame buffers for each device
    initTransferFrameSendBuffers();

    tmTcTarget = objectManager->get<RS485TmTcTarget>(tmTcTargetId);
    if (tmTcTarget == nullptr) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    ReturnValue_t result = tmTcTarget->setvirtualChannelFrameSizes(&virtualChannelFrameSizes);
    return result;
}

ReturnValue_t RS485DeviceComIF::initializeInterface(CookieIF *cookie) {
    if (cookie != nullptr) {
        RS485Cookie *rs485Cookie = dynamic_cast<RS485Cookie*>(cookie);
        RS485Devices device = rs485Cookie->getDevice();
        deviceCookies[device] = cookie;
        //TODO: Returnvalue for map insertion
        virtualChannelFrameSizes.insert(
                std::pair<uint8_t, size_t>(rs485Cookie->getVcId(),
                        rs485Cookie->getTfdzSize() + sendBuffer[device]->FRAME_OVERHEAD));
        return HasReturnvaluesIF::RETURN_OK;
    } else {
        sif::error << "RS485DeviceComIF::initializeInterface failed: Cookie is null pointer"
                << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }

}

ReturnValue_t RS485DeviceComIF::performOperation(uint8_t opCode) {

    RS485Devices device = static_cast<RS485Devices>(opCode);

    if (deviceCookies[device] != nullptr) {
        // TODO: Make sure this does not turn into a bad cast as we have no exceptions
        RS485Cookie *rs485Cookie = dynamic_cast<RS485Cookie*>(deviceCookies[device]);
        // This switch only handles the correct GPIO for Transceivers
        switch (device) {
        case (RS485Devices::COM_FPGA): {
            // TODO: Check which FPGA is active (should probably be set via DeviceHandler in Cookie)
#ifdef DEBUG
			sif::info << "Sending to FPGA" << std::endl;
#endif
            GpioDeviceComIF::enableTransceiverFPGA1();
            // Normal device command to CCSDS board still need to be sent
            handleSend(device, rs485Cookie);
            handleTmSend(device, rs485Cookie);
            break;
        }
        case (RS485Devices::PCDU_VORAGO): {
#ifdef DEBUG
			sif::info << "Sending to PCDU" << std::endl;
#endif
            GpioDeviceComIF::enableTransceiverPCDU();
            handleSend(device, rs485Cookie);
            break;
        }
        case (RS485Devices::PL_VORAGO): {
#ifdef DEBUG
			sif::info << "Sending to PL_VORAGO" << std::endl;
#endif
            handleSend(device, rs485Cookie);
            GpioDeviceComIF::enableTransceiverVorago();
            break;
        }
        case (RS485Devices::PL_PIC24): {
#ifdef DEBUG
			sif::info << "Sending to PL_PIC24" << std::endl;
#endif
            handleSend(device, rs485Cookie);
            GpioDeviceComIF::enableTransceiverPIC24();
            break;
        }
        default: {
#ifdef DEBUG
			// should not happen
			// TODO: Is there anything special to do in this case
			sif::error << "RS485 Device Number out of bounds" << std::endl;
#endif
            break;
        }
        }

    } else {
        sif::error << "RS485 Device Cookies not initialized yet" << std::endl;
    }

    if (retryCount > 0) {
#ifdef DEBUG
        sif::error << "RS485DeviceComIF::performOperation: RS485DeviceComIF"
                << " driver was busy for " << (uint16_t) retryCount
                << " attempts!" << std::endl;
#endif
        retryCount = 0;
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t RS485DeviceComIF::sendMessage(CookieIF *cookie, const uint8_t *sendData,
        size_t sendLen) {

    RS485Cookie *rs485Cookie = dynamic_cast<RS485Cookie*>(cookie);
    RS485Devices device = rs485Cookie->getDevice();

    // Check if there already is a message that has not been processed yet
    if (rs485Cookie->getComStatus() == ComStatusRS485::IDLE) {
        // Copy Message into corresponding sendFrameBuffer
        (void) std::memcpy(sendBuffer[device]->getDataZone(), sendData, sendLen);
        rs485Cookie->setComStatus(ComStatusRS485::TRANSFER_INIT_SUCCESS);
        // Resets return value from last transfer
        rs485Cookie->setReturnValue(0);

        return HasReturnvaluesIF::RETURN_OK;
    } else {
        sif::error << "RS485DeviceComIF::sendMessage: Device queue full" << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }
}

ReturnValue_t RS485DeviceComIF::getSendSuccess(CookieIF *cookie) {

    RS485Cookie *rs485Cookie = dynamic_cast<RS485Cookie*>(cookie);
    // Com Status is only reset here, so this always has to be called
    // after sendMessage()
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
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t RS485DeviceComIF::readReceivedMessage(CookieIF *cookie, uint8_t **buffer,
        size_t *size) {
    return HasReturnvaluesIF::RETURN_OK;
}

std::map<uint8_t, size_t>* RS485DeviceComIF::getVcidSizeMap() {
    return &virtualChannelFrameSizes;
}

void RS485DeviceComIF::initTransferFrameSendBuffers() {

    // Buffer construction with different TFDZ sizes
    sendBuffer[RS485Devices::COM_FPGA] = new USLPTransferFrame(transmitBufferFPGA.data(),
            config::RS485_COM_FPGA_TFDZ_SIZE);
    sendBuffer[RS485Devices::PCDU_VORAGO] = new USLPTransferFrame(transmitBufferPCDU.data(),
            config::RS485_PCDU_VORAGO_TFDZ_SIZE);
    sendBuffer[RS485Devices::PL_VORAGO] = new USLPTransferFrame(transmitBufferVorago.data(),
            config::RS485_PAYLOAD_VORAGO_TFDZ_SIZE);
    sendBuffer[RS485Devices::PL_PIC24] = new USLPTransferFrame(transmitBufferPIC24.data(),
            config::RS485_PAYLOAD_PIC24_TFDZ_SIZE);

    // Default values, most don't change
    for (int i = 0; i < RS485Devices::DEVICE_COUNT_RS485; i++) {
        sendBuffer[i]->setVersionNumber(config::RS485_USLP_TFVN);
        sendBuffer[i]->setSpacecraftId(config::RS485_USLP_SCID);
        sendBuffer[i]->setSourceFlag(true);
        sendBuffer[i]->setVirtualChannelId(0);
        sendBuffer[i]->setMapId(1);
        sendBuffer[i]->setTruncatedFlag(true);
        sendBuffer[i]->setTFDZConstructionRules(0);
        sendBuffer[i]->setProtocolIdentifier(0);
        sendBuffer[i]->setFirstHeaderOffset(0);

    }
    sendBuffer[RS485Devices::PCDU_VORAGO]->setVirtualChannelId(1);
    sendBuffer[RS485Devices::PL_PIC24]->setVirtualChannelId(2);
    sendBuffer[RS485Devices::PL_VORAGO]->setVirtualChannelId(3);
    //TODO: Correct VCID and MAP ID for each buffer

}
void RS485DeviceComIF::handleSend(RS485Devices device, RS485Cookie *rs485Cookie) {

    // This condition sets the buffer to default messages if there is no message
    // or if last communication was faulty
    if (rs485Cookie->getComStatus() != ComStatusRS485::TRANSFER_INIT_SUCCESS) {
        (void) std::memcpy(sendBuffer[device]->getDataZone(), defaultMessage,
                sizeof(defaultMessage));
    }

    // Set address just to be sure it was not changed
    sendBuffer[device]->setVirtualChannelId(rs485Cookie->getVcId());
    sendBuffer[device]->setMapId(rs485Cookie->getMapId());
    // Buffer is already filled, so just send it
    int retval = UART_write(bus2_uart, sendBuffer[device]->getFullFrame(),
            sendBuffer[device]->getFullFrameSize());

    //TODO:  We  memset here but USLP wants encapsulation idle packet according to
    // CCSDS 133.1-B-2, additionally there have been problems with non-randomized idle data
    // so we will replace this later
    memset(sendBuffer[device]->getDataZone(), 0, sendBuffer[device]->getDataZoneSize());

    //TODO: Mutex for ComStatus
    rs485Cookie->setReturnValue(retval);
    if (retval != 0) {
        rs485Cookie->setComStatus(ComStatusRS485::FAULTY);
    } else {
        rs485Cookie->setComStatus(ComStatusRS485::TRANSFER_SUCCESS);
    }

}

void RS485DeviceComIF::handleTmSend(RS485Devices device, RS485Cookie *rs485Cookie) {

    // TODO: Check if downlink available
    for (packetSentCounter = 0;
            tmTcTarget->fillSendFrameBuffer(sendBuffer[device]) == HasReturnvaluesIF::RETURN_OK;
            packetSentCounter++) {

        // TODO: Integrate this into cookie
        sendBuffer[device]->setMapId(config::RS485_USLP_MAPID_COM_FPGA_1_TM);
        // TODO: do something with result
        UART_write(bus2_uart, sendBuffer[device]->getFullFrame(),
                sendBuffer[device]->getFullFrameSize());

        //TODO:  We  memset here but USLP wants encapsulation idle packet according to
        // CCSDS 133.1-B-2, additionally there have been problems with non-randomized idle data
        // so we will replace this later
        memset(sendBuffer[device]->getDataZone(), 0, sendBuffer[device]->getDataZoneSize());

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
#ifdef DEBUG
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

