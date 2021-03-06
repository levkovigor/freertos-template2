#include "ThermalSensorHandler.h"

#include <bitset>
#include <cmath>

ThermalSensorHandler::ThermalSensorHandler(object_id_t objectId,
        object_id_t comIF, CookieIF *comCookie, uint8_t switchId):
        DeviceHandlerBase(objectId, comIF, comCookie), switchId(switchId),
        sensorDataset(this), sensorDatasetSid(sensorDataset.getSid()) {
#if OBSW_VERBOSE_LEVEL >= 1
    debugDivider = new PeriodicOperationDivider(10);
#endif
}

ThermalSensorHandler::~ThermalSensorHandler() {
#if OBSW_VERBOSE_LEVEL >= 1
    delete(debugDivider);
#endif
}

void ThermalSensorHandler::doStartUp() {
    if(internalState == InternalState::NONE) {
        internalState = InternalState::WARMUP;
        Clock::getUptime(&startTime);
    }

    if(internalState == InternalState::WARMUP) {
        dur_millis_t timeNow = 0;
        Clock::getUptime(&timeNow);
        if(timeNow - startTime >= 100) {
            internalState = InternalState::CONFIGURE;
        }
    }

    if(internalState == InternalState::CONFIGURE) {
        if(commandExecuted) {
            internalState = InternalState::REQUEST_CONFIG;
            commandExecuted = false;
        }
    }

    if(internalState == InternalState::REQUEST_CONFIG) {
        if (commandExecuted) {
            setMode(MODE_ON);
            setMode(MODE_NORMAL);
            commandExecuted = false;
            internalState = InternalState::RUNNING;
        }
    }
}

void ThermalSensorHandler::doShutDown() {
    commandExecuted = false;
    setMode(MODE_OFF);
}

ReturnValue_t ThermalSensorHandler::buildNormalDeviceCommand(
        DeviceCommandId_t *id) {
    if(internalState == InternalState::RUNNING) {
        *id = TSensorDefinitions::REQUEST_RTD;
        return buildCommandFromCommand(*id, nullptr, 0);
    }
    else if(internalState == InternalState::REQUEST_FAULT_BYTE) {
        *id = TSensorDefinitions::REQUEST_FAULT_BYTE;
        return buildCommandFromCommand(*id, nullptr, 0);
    }
    else {
        return DeviceHandlerBase::NOTHING_TO_SEND;
    }
}

ReturnValue_t ThermalSensorHandler::buildTransitionDeviceCommand(
        DeviceCommandId_t *id) {
    switch(internalState) {
    case(InternalState::NONE):
    case(InternalState::WARMUP):
    case(InternalState::RUNNING):
    return DeviceHandlerBase::NOTHING_TO_SEND;
    case(InternalState::CONFIGURE): {
        *id = TSensorDefinitions::CONFIG_CMD;
        uint8_t config[1] = {DEFAULT_CONFIG};
        return buildCommandFromCommand(*id, config, 1);
    }
    case(InternalState::REQUEST_CONFIG): {
        *id = TSensorDefinitions::REQUEST_CONFIG;
        return buildCommandFromCommand(*id, nullptr, 0);
    }

    default:
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "ThermalSensorHandler: Invalid internal state" << std::endl;
#else
        sif::printError("ThermalSensorHandler: Invalid internal state\n");
#endif
        return HasReturnvaluesIF::RETURN_FAILED;
    }
}

ReturnValue_t ThermalSensorHandler::buildCommandFromCommand(
        DeviceCommandId_t deviceCommand, const uint8_t *commandData,
        size_t commandDataLen) {
    switch(deviceCommand) {
    case(TSensorDefinitions::CONFIG_CMD) : {
        commandBuffer[0] = static_cast<uint8_t>(TSensorDefinitions::CONFIG_CMD);
        if(commandDataLen == 1) {
            commandBuffer[1] = commandData[0];
            DeviceHandlerBase::rawPacketLen = 2;
            DeviceHandlerBase::rawPacket = commandBuffer.data();
            return HasReturnvaluesIF::RETURN_OK;
        }
        else {
            return DeviceHandlerIF::NO_COMMAND_DATA;
        }
    }
    case(TSensorDefinitions::REQUEST_CONFIG): {
        commandBuffer[0] = 0x00; // dummy byte
        commandBuffer[1] = static_cast<uint8_t>(
                TSensorDefinitions::REQUEST_CONFIG);
        DeviceHandlerBase::rawPacketLen = 2;
        DeviceHandlerBase::rawPacket = commandBuffer.data();
        return HasReturnvaluesIF::RETURN_OK;
    }
    case(TSensorDefinitions::REQUEST_RTD): {
        commandBuffer[0] = static_cast<uint8_t>(
                TSensorDefinitions::REQUEST_RTD);
        // two dummy bytes
        commandBuffer[1] = 0x00;
        commandBuffer[2] = 0x00;
        DeviceHandlerBase::rawPacketLen = 3;
        DeviceHandlerBase::rawPacket = commandBuffer.data();
        return HasReturnvaluesIF::RETURN_OK;
    }
    case(TSensorDefinitions::REQUEST_FAULT_BYTE): {
        commandBuffer[0] = static_cast<uint8_t>(
                TSensorDefinitions::REQUEST_FAULT_BYTE);
        commandBuffer[1] = 0x00;
        DeviceHandlerBase::rawPacketLen = 2;
        DeviceHandlerBase::rawPacket = commandBuffer.data();
        return HasReturnvaluesIF::RETURN_OK;
    }
    default:
        //Unknown DeviceCommand
        return DeviceHandlerIF::COMMAND_NOT_IMPLEMENTED;
    }
}

void ThermalSensorHandler::fillCommandAndReplyMap() {
    insertInCommandAndReplyMap(TSensorDefinitions::CONFIG_CMD, 3);
    insertInCommandAndReplyMap(TSensorDefinitions::REQUEST_CONFIG, 3);
    insertInCommandAndReplyMap(TSensorDefinitions::REQUEST_RTD, 3,
            &sensorDataset);
    insertInCommandAndReplyMap(TSensorDefinitions::REQUEST_FAULT_BYTE, 3);
}

ReturnValue_t ThermalSensorHandler::scanForReply(const uint8_t *start,
        size_t remainingSize, DeviceCommandId_t *foundId, size_t *foundLen) {
    size_t rtdReplySize = 3;
    size_t configReplySize = 2;

    if(remainingSize == rtdReplySize and
            internalState == InternalState::RUNNING) {
        *foundId = TSensorDefinitions::REQUEST_RTD;
        *foundLen = rtdReplySize;
    }

    if(remainingSize == configReplySize) {
        if(internalState == InternalState::CONFIGURE) {
            commandExecuted = true;
            *foundLen = configReplySize;
            *foundId = TSensorDefinitions::CONFIG_CMD;
        }
        else if(internalState == InternalState::REQUEST_FAULT_BYTE) {
            *foundId = TSensorDefinitions::REQUEST_FAULT_BYTE;
            *foundLen = 2;
            internalState = InternalState::RUNNING;
        }
        else {
            *foundId = TSensorDefinitions::REQUEST_CONFIG;
            *foundLen = configReplySize;
        }
    }

    return RETURN_OK;
}

ReturnValue_t ThermalSensorHandler::interpretDeviceReply(
        DeviceCommandId_t id, const uint8_t *packet) {
    switch(id) {
    case(TSensorDefinitions::REQUEST_CONFIG): {
        if(packet[1] != DEFAULT_CONFIG) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            // it propably would be better if we at least try one restart..
            sif::error << "ThermalSensorHandler: Invalid configuration reply!" << std::endl;
#else
            sif::printError("ThermalSensorHandler: Invalid configuration reply!\n");
#endif
            return HasReturnvaluesIF::RETURN_OK;
        }
        // set to true for invalid configs too for now.
        if(internalState == InternalState::REQUEST_CONFIG) {
            commandExecuted = true;
        }
        else if(internalState == InternalState::RUNNING) {
            // we should propably generate a telemetry with the config byte
            // as payload here.
        }
        break;
    }
    case(TSensorDefinitions::REQUEST_RTD): {
        // first bit of LSB reply byte is the fault bit
        uint8_t faultBit = packet[2] & 0b0000'0001;
        if(faultBit == 1) {
            // Maybe we should attempt to restart it?
            if(faultByte == 0) {
                internalState = InternalState::REQUEST_FAULT_BYTE;
            }
        }

        // RTD value consists of last seven bits of the LSB reply byte and
        // the MSB reply byte
        uint16_t adcCode = ((packet[1] << 8) | packet[2]) >> 1;
        // do something with rtd value, will propably be stored in
        // dataset.
        float rtdValue = adcCode * RTD_RREF_PT1000 / INT16_MAX;

        // calculate approximation
        float approxTemp = adcCode / 32.0 - 256.0;

#if OBSW_VERBOSE_LEVEL >= 1
        if(debugDivider->checkAndIncrement()) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << "ThermalSensorHandler::interpretDeviceReply: Measured "
                    << "resistance is " << rtdValue << " Ohms." << std::endl;
            sif::info << "Approximated temperature is " << approxTemp << " ?C"
                    << std::endl;
#else
            sif::printInfo("ThermalSensorHandler::interpretDeviceReply: Measured resistance is %f"
                    " Ohms.\n", rtdValue);
            sif::printInfo("Approximated temperature is %f C\n", approxTemp);
#endif
            sensorDataset.setChanged(true);
        }
#endif

        ReturnValue_t result = sensorDataset.read();
        if(result != HasReturnvaluesIF::RETURN_OK) {
            // Configuration error
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::debug << "ThermalSensorHandler::interpretDeviceReply: Error reading dataset!"
                    << std::endl;
#else
            sif::printDebug("ThermalSensorHandler::interpretDeviceReply: Error reading dataset!\n");
#endif
            return result;
        }

        if(not sensorDataset.isValid()) {
            sensorDataset.temperatureCelcius.setValid(true);
        }

        sensorDataset.temperatureCelcius = approxTemp;

        result = sensorDataset.commit();

        if(result != HasReturnvaluesIF::RETURN_OK) {
            // Configuration error
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::debug << "ThermalSensorHandler::interpretDeviceReply: "
                    "Error commiting dataset!" << std::endl;
#else
            sif::printDebug("ThermalSensorHandler::interpretDeviceReply: "
                    "Error commiting dataset!\n");
#endif
            return result;
        }

        break;
    }
    case(TSensorDefinitions::REQUEST_FAULT_BYTE): {
        faultByte = packet[1];
#if OBSW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "ThermalSensorHandler::interpretDeviceReply: Fault byte"
                " is: 0b" << std::bitset<8>(faultByte) << std::endl;
#else
        sif::printInfo("ThermalSensorHandler::interpretDeviceReply: Fault byte"
                " is: 0b" BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(faultByte));
#endif
#endif
        ReturnValue_t result = sensorDataset.read();
        if(result != HasReturnvaluesIF::RETURN_OK) {
            // Configuration error
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::debug << "ThermalSensorHandler::interpretDeviceReply: "
                    "Error reading dataset!" << std::endl;
#else
            sif::printDebug("ThermalSensorHandler::interpretDeviceReply: "
                    "Error reading dataset!\n");
#endif
            return result;
        }
        sensorDataset.errorByte.setValid(true);
        sensorDataset.errorByte = faultByte;
        if(faultByte != 0) {
            sensorDataset.temperatureCelcius.setValid(false);
        }

        result = sensorDataset.commit();
        if(result != HasReturnvaluesIF::RETURN_OK) {
            // Configuration error
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::debug << "ThermalSensorHandler::interpretDeviceReply: "
                    "Error commiting dataset!" << std::endl;
#else
            sif::printDebug("ThermalSensorHandler::interpretDeviceReply: "
                    "Error commiting dataset!\n");
#endif
            return result;
        }

        break;
    }
    default:
        break;
    }
    return HasReturnvaluesIF::RETURN_OK;
}

void ThermalSensorHandler::debugInterface(uint8_t positionTracker,
        object_id_t objectId, uint32_t parameter) {
}

uint32_t ThermalSensorHandler::getTransitionDelayMs(
        Mode_t modeFrom, Mode_t modeTo) {
    return 5000;
}

ReturnValue_t ThermalSensorHandler::getSwitches(
        const uint8_t **switches, uint8_t *numberOfSwitches) {
    return DeviceHandlerBase::NO_SWITCH;
}

void ThermalSensorHandler::doTransition(Mode_t modeFrom,
        Submode_t subModeFrom) {
    DeviceHandlerBase::doTransition(modeFrom, subModeFrom);
}

ReturnValue_t ThermalSensorHandler::initializeLocalDataPool(localpool::DataPool& localDataPoolMap,
        LocalDataPoolManager& poolManager) {
    localDataPoolMap.emplace(TSensorDefinitions::PoolIds::TEMPERATURE_C,
            new PoolEntry<float>({0}, 1, true));
    localDataPoolMap.emplace(TSensorDefinitions::PoolIds::FAULT_BYTE,
            new PoolEntry<uint8_t>({0}));
    poolManager.subscribeForPeriodicPacket(sensorDatasetSid,
            false, 4.0, false);
    return HasReturnvaluesIF::RETURN_OK;
}

void ThermalSensorHandler::modeChanged() {
    internalState = InternalState::NONE;
}
