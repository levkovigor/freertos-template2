#include "MGMHandlerLIS3MDL.h"


MGMHandlerLIS3MDL::MGMHandlerLIS3MDL(object_id_t objectId,
        object_id_t deviceCommunication, CookieIF* comCookie):
		DeviceHandlerBase(objectId, deviceCommunication, comCookie) {
#if OBSW_ENHANCED_PRINTOUT == 1
    debugDivider = new PeriodicOperationDivider(10);
#endif
    // Set to default values right away.
    registers[0] = MGMLIS3MDL::CTRL_REG1_DEFAULT;
    registers[1] = MGMLIS3MDL::CTRL_REG2_DEFAULT;
    registers[2] = MGMLIS3MDL::CTRL_REG3_DEFAULT;
    registers[3] = MGMLIS3MDL::CTRL_REG4_DEFAULT;
    registers[4] = MGMLIS3MDL::CTRL_REG5_DEFAULT;

}

MGMHandlerLIS3MDL::~MGMHandlerLIS3MDL() {
}


void MGMHandlerLIS3MDL::doStartUp() {
    switch (internalState) {
    case STATE_NONE:
        internalState = STATE_FIRST_CONTACT;
        break;

    case STATE_FIRST_CONTACT:
        internalState = STATE_SETUP;
        break;

    case STATE_SETUP:
        internalState = STATE_CHECK_REGISTERS;
        break;

    case STATE_CHECK_REGISTERS: {
        // Set up cached registers which will be used to configure the MGM.
        if(commandExecuted) {
            commandExecuted = false;
            setMode(MODE_NORMAL);
        }
        break;
    }
    default:
        break;
    }

}

void MGMHandlerLIS3MDL::doShutDown() {
    setMode(_MODE_POWER_DOWN);
}

ReturnValue_t MGMHandlerLIS3MDL::buildTransitionDeviceCommand(
        DeviceCommandId_t *id) {
    switch (internalState) {
    case STATE_FIRST_CONTACT:
        *id = MGMLIS3MDL::IDENTIFY_DEVICE;
        break;

    case STATE_SETUP:
        *id = MGMLIS3MDL::SETUP_MGM;
        break;

    case STATE_CHECK_REGISTERS:
        *id = MGMLIS3MDL::READ_CONFIG_AND_DATA;
        break;

    default:
        break;
    }
    return buildCommandFromCommand(*id, NULL, 0);
}

uint8_t MGMHandlerLIS3MDL::readCommand(uint8_t command, bool continuousCom) {
	command |= (1 << MGMLIS3MDL::RW_BIT);
	if (continuousCom == true) {
		command |= (1 << MGMLIS3MDL::MS_BIT);
	}
	return command;
}

uint8_t MGMHandlerLIS3MDL::writeCommand(uint8_t command, bool continuousCom) {
	command &= ~(1 << MGMLIS3MDL::RW_BIT);
	if (continuousCom == true) {
		command |= (1 << MGMLIS3MDL::MS_BIT);
	}
	return command;
}

void MGMHandlerLIS3MDL::setupMgm() {

	registers[0] = MGMLIS3MDL::CTRL_REG1_DEFAULT;
	registers[1] = MGMLIS3MDL::CTRL_REG2_DEFAULT;
	registers[2] = MGMLIS3MDL::CTRL_REG3_DEFAULT;
	registers[3] = MGMLIS3MDL::CTRL_REG4_DEFAULT;
	registers[4] = MGMLIS3MDL::CTRL_REG5_DEFAULT;

	prepareCtrlRegisterWrite();
}

ReturnValue_t MGMHandlerLIS3MDL::buildNormalDeviceCommand(
		DeviceCommandId_t *id) {
    // Data/config register will be read in an alternating manner.
    if(communicationStep == CommunicationStep::DATA) {
        lastSentCommand = MGMLIS3MDL::READ_CONFIG_AND_DATA;
        *id = MGMLIS3MDL::READ_CONFIG_AND_DATA;
        communicationStep = CommunicationStep::TEMPERATURE;
        return buildCommandFromCommand(*id, NULL, 0);
    }
    else {
        lastSentCommand = MGMLIS3MDL::READ_TEMPERATURE;
        *id = MGMLIS3MDL::READ_TEMPERATURE;
        communicationStep = CommunicationStep::DATA;
        return buildCommandFromCommand(*id, NULL, 0);
    }

}

ReturnValue_t MGMHandlerLIS3MDL::buildCommandFromCommand(
        DeviceCommandId_t deviceCommand, const uint8_t *commandData,
		size_t commandDataLen) {
    lastSentCommand = deviceCommand;
    switch(deviceCommand) {
    case(MGMLIS3MDL::READ_CONFIG_AND_DATA): {
        std::memset(commandBuffer, 0, sizeof(commandBuffer));
        commandBuffer[0] = readCommand(MGMLIS3MDL::CTRL_REG1, true);

        rawPacket = commandBuffer;
        rawPacketLen = MGMLIS3MDL::NR_OF_DATA_AND_CFG_REGISTERS + 1;
        return RETURN_OK;
    }
    case(MGMLIS3MDL::READ_TEMPERATURE): {
        std::memset(commandBuffer, 0, 3);
        commandBuffer[0] = readCommand(MGMLIS3MDL::TEMP_LOWBYTE, true);

        rawPacket = commandBuffer;
        rawPacketLen = 3;
        return RETURN_OK;
    }
	case(MGMLIS3MDL::IDENTIFY_DEVICE): {
	    return identifyDevice();
	}
	case(MGMLIS3MDL::TEMP_SENSOR_ENABLE): {
	    return enableTemperatureSensor(commandData, commandDataLen);
	}
	case(MGMLIS3MDL::SETUP_MGM): {
	    setupMgm();
	    return HasReturnvaluesIF::RETURN_OK;
	}
	case(MGMLIS3MDL::ACCURACY_OP_MODE_SET): {
	    return setOperatingMode(commandData, commandDataLen);
	}
	default:
	    lastSentCommand = DeviceHandlerIF::NO_COMMAND;
	    return DeviceHandlerIF::COMMAND_NOT_IMPLEMENTED;
	}
	return HasReturnvaluesIF::RETURN_FAILED;
}

ReturnValue_t MGMHandlerLIS3MDL::identifyDevice() {
    uint32_t size = 2;
    commandBuffer[0] = readCommand(MGMLIS3MDL::IDENTIFY_DEVICE_REG_ADDR);
    commandBuffer[1] = 0x00;

    rawPacket = commandBuffer;
    rawPacketLen = size;

    return RETURN_OK;
}

ReturnValue_t MGMHandlerLIS3MDL::scanForReply(const uint8_t *start,
		size_t len, DeviceCommandId_t *foundId, size_t *foundLen) {
	*foundLen = len;
	if (len == MGMLIS3MDL::NR_OF_DATA_AND_CFG_REGISTERS + 1) {
		*foundLen = len;
		*foundId = MGMLIS3MDL::READ_CONFIG_AND_DATA;
		// Check validity by checking config registers
		if (start[1] != registers[0] or start[2] != registers[1] or
		        start[3] != registers[2] or start[4] != registers[3] or
		        start[5] != registers[4]) {
			return DeviceHandlerIF::INVALID_DATA;
		}
        if(mode == _MODE_START_UP) {
            commandExecuted = true;
        }

	}
	else if(len == MGMLIS3MDL::TEMPERATURE_REPLY_LEN) {
	    *foundLen = len;
	    *foundId = MGMLIS3MDL::READ_TEMPERATURE;
	}
	else if (len == MGMLIS3MDL::SETUP_REPLY_LEN) {
		*foundLen = len;
		*foundId = MGMLIS3MDL::SETUP_MGM;
	}
	else if (len == SINGLE_COMMAND_ANSWER_LEN) {
		*foundLen = len;
		*foundId = lastSentCommand;
	}
	else {
		return DeviceHandlerIF::INVALID_DATA;
	}

	// Data with SPI Interface has always this answer
	if (start[0] == 0b11111111) {
		return RETURN_OK;
	}
	else {
		return DeviceHandlerIF::INVALID_DATA;
	}

}
ReturnValue_t MGMHandlerLIS3MDL::interpretDeviceReply(DeviceCommandId_t id,
		const uint8_t *packet) {

	switch (id) {
	case MGMLIS3MDL::IDENTIFY_DEVICE: {
		break;
	}
	case MGMLIS3MDL::SETUP_MGM: {
		break;
	}
	case MGMLIS3MDL::READ_CONFIG_AND_DATA: {
	    // TODO: Store configuration and sensor values in new local datasets.

		uint8_t scale = getFullScale(registers[2]);
		float sensitivityFactor = getSensitivityFactor(scale);

		int16_t mgmMeasurementRawX = packet[MGMLIS3MDL::X_HIGHBYTE_IDX] << 8
		        | packet[MGMLIS3MDL::X_LOWBYTE_IDX] ;
        int16_t mgmMeasurementRawY = packet[MGMLIS3MDL::Y_HIGHBYTE_IDX] << 8
                | packet[MGMLIS3MDL::Y_LOWBYTE_IDX] ;
        int16_t mgmMeasurementRawZ = packet[MGMLIS3MDL::Z_HIGHBYTE_IDX] << 8
                | packet[MGMLIS3MDL::Z_LOWBYTE_IDX] ;

		// Target value in microtesla
		float mgmX = static_cast<float>(mgmMeasurementRawX) * sensitivityFactor
		        *  MGMLIS3MDL::GAUSS_TO_MICROTESLA_FACTOR;
		float mgmY = static_cast<float>(mgmMeasurementRawY) * sensitivityFactor
		        *  MGMLIS3MDL::GAUSS_TO_MICROTESLA_FACTOR;
		float mgmZ = static_cast<float>(mgmMeasurementRawZ) * sensitivityFactor
		        *  MGMLIS3MDL::GAUSS_TO_MICROTESLA_FACTOR;

#if OBSW_ENHANCED_PRINTOUT == 1
		if(debugDivider->checkAndIncrement()) {
            sif::info << "MGMHandlerLIS3: Magnetic field strength in"
                    " microtesla:" << std::endl;
            // Set terminal to utf-8 if there is an issue with micro printout.
            sif::info << "X: " << mgmX << " \xC2\xB5T" << std::endl;
            sif::info << "Y: " << mgmY << " \xC2\xB5T" << std::endl;
            sif::info << "Z: " << mgmZ << " \xC2\xB5T" << std::endl;
		}
#endif
		break;
	}

	case MGMLIS3MDL::READ_TEMPERATURE: {
	    int16_t tempValueRaw = packet[2] << 8 | packet[1];
        float tempValue = 25.0 + ((static_cast<float>(tempValueRaw)) / 8.0);
#if OBSW_ENHANCED_PRINTOUT == 1
        if(debugDivider->check()) {
            // Set terminal to utf-8 if there is an issue with micro printout.
            sif::info << "MGMHandlerLIS3: Temperature: " << tempValue<< " °C"
                    << std::endl;
        }
#endif
        break;
	}

	default: {
		return DeviceHandlerIF::UNKNOW_DEVICE_REPLY;
	}

	}
	return RETURN_OK;
}

uint8_t MGMHandlerLIS3MDL::getFullScale(uint8_t ctrlRegister2) {
	bool FS0 = false;
	bool FS1 = false;
	if ((ctrlRegister2 >> 5) == 1)
		FS0 = true;
	if ((ctrlRegister2 >> 6) == 1)
		FS1 = true;
	if ((FS0 == true) && (FS1 == true))
		return 16;
	else if ((FS0 == false) && (FS1 == true))
		return 12;
	else if ((FS0 == true) && (FS1 == false))
		return 8;
	else
		return 4;
}

float MGMHandlerLIS3MDL::getSensitivityFactor(uint8_t scale) {
	return (float) scale / (INT16_MAX);
}


ReturnValue_t MGMHandlerLIS3MDL::enableTemperatureSensor(
		const uint8_t *commandData, size_t commandDataLen) {
	triggerEvent(CHANGE_OF_SETUP_PARAMETER);
	uint32_t size = 2;
	commandBuffer[0] = writeCommand(MGMLIS3MDL::CTRL_REG1);
	if (commandDataLen > 1) {
		return INVALID_NUMBER_OR_LENGTH_OF_PARAMETERS;
	}
	switch (*commandData) {
	case (MGMLIS3MDL::ON):
		commandBuffer[1] = registers[0] | (1 << 7);
		break;

	case (MGMLIS3MDL::OFF):
		commandBuffer[1] = registers[0] & ~(1 << 7);
		break;

	default:
		return INVALID_COMMAND_PARAMETER;
		break;
	}
	registers[0] = commandBuffer[1];

	rawPacket = commandBuffer;
	rawPacketLen = size;

	return RETURN_OK;
}

ReturnValue_t MGMHandlerLIS3MDL::setOperatingMode(const uint8_t *commandData,
		size_t commandDataLen) {
	triggerEvent(CHANGE_OF_SETUP_PARAMETER);
	if (commandDataLen != 1) {
		return INVALID_NUMBER_OR_LENGTH_OF_PARAMETERS;
	}

	switch (commandData[0]) {
	case MGMLIS3MDL::LOW:
		registers[0] = (registers[0] & (~(1 << MGMLIS3MDL::OM1))) & (~(1 << MGMLIS3MDL::OM0));
		registers[3] = (registers[3] & (~(1 << MGMLIS3MDL::OMZ1))) & (~(1 << MGMLIS3MDL::OMZ0));
		break;
	case MGMLIS3MDL::MEDIUM:
		registers[0] = (registers[0] & (~(1 << MGMLIS3MDL::OM1))) | (1 << MGMLIS3MDL::OM0);
		registers[3] = (registers[3] & (~(1 << MGMLIS3MDL::OMZ1))) | (1 << MGMLIS3MDL::OMZ0);
		break;

	case MGMLIS3MDL::HIGH:
		registers[0] = (registers[0] | (1 << MGMLIS3MDL::OM1)) & (~(1 << MGMLIS3MDL::OM0));
		registers[3] = (registers[3] | (1 << MGMLIS3MDL::OMZ1)) & (~(1 << MGMLIS3MDL::OMZ0));
		break;

	case MGMLIS3MDL::ULTRA:
		registers[0] = (registers[0] | (1 << MGMLIS3MDL::OM1)) | (1 << MGMLIS3MDL::OM0);
		registers[3] = (registers[3] | (1 << MGMLIS3MDL::OMZ1)) | (1 << MGMLIS3MDL::OMZ0);
		break;
	default:
		break;
	}

	return prepareCtrlRegisterWrite();
}

void MGMHandlerLIS3MDL::fillCommandAndReplyMap() {
	/*
	 * Regarding ArduinoBoard:
	 * Actually SPI answers directly, but as commanding ArduinoBoard the
	 * communication could be delayed
	 * SPI always has to be triggered, so there could be no periodic answer of
	 * the device, the device has to asked with a command, so periodic is zero.
	 *
	 * We dont read single registers, we just expect special
	 * reply from he Readall_MGM
	 */
	insertInCommandAndReplyMap(MGMLIS3MDL::READ_CONFIG_AND_DATA, 1);
	insertInCommandAndReplyMap(MGMLIS3MDL::READ_TEMPERATURE, 1);
	insertInCommandAndReplyMap(MGMLIS3MDL::SETUP_MGM, 1);
	insertInCommandAndReplyMap(MGMLIS3MDL::IDENTIFY_DEVICE, 1);
	insertInCommandAndReplyMap(MGMLIS3MDL::TEMP_SENSOR_ENABLE, 1);
	insertInCommandAndReplyMap(MGMLIS3MDL::ACCURACY_OP_MODE_SET, 1);
}

ReturnValue_t MGMHandlerLIS3MDL::prepareCtrlRegisterWrite() {

	commandBuffer[0] = writeCommand(MGMLIS3MDL::CTRL_REG1, true);

	for (size_t i = 0; i < MGMLIS3MDL::NR_OF_CTRL_REGISTERS; i++) {
		commandBuffer[i + 1] = registers[i];
	}
	rawPacket = commandBuffer;
	rawPacketLen = MGMLIS3MDL::NR_OF_CTRL_REGISTERS + 1;

	// We dont have to check if this is working because we just did it
	return RETURN_OK;
}

void MGMHandlerLIS3MDL::setNormalDatapoolEntriesInvalid() {
    // TODO: use new distributed datapools here.
}

void MGMHandlerLIS3MDL::doTransition(Mode_t modeFrom, Submode_t subModeFrom) {

}

uint32_t MGMHandlerLIS3MDL::getTransitionDelayMs(Mode_t from, Mode_t to) {
    return 5000;
}

void MGMHandlerLIS3MDL::modeChanged(void) {
	internalState = STATE_NONE;
}

ReturnValue_t MGMHandlerLIS3MDL::initializeLocalDataPool(
        LocalDataPool &localDataPoolMap, LocalDataPoolManager &poolManager) {
    localDataPoolMap.emplace(MGMLIS3MDL::FIELD_STRENGTH_X,
            new PoolEntry<float>({0.0}));
    localDataPoolMap.emplace(MGMLIS3MDL::FIELD_STRENGTH_Y,
            new PoolEntry<float>({0.0}));
    localDataPoolMap.emplace(MGMLIS3MDL::FIELD_STRENGTH_Z,
            new PoolEntry<float>({0.0}));
    localDataPoolMap.emplace(MGMLIS3MDL::TEMPERATURE_CELCIUS,
            new PoolEntry<float>({0.0}));
    return HasReturnvaluesIF::RETURN_OK;
}
