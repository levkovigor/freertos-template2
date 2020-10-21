#include "MGMHandlerLIS3MDL.h"

MGMHandlerLIS3MDL::MGMHandlerLIS3MDL(object_id_t objectId,
        object_id_t deviceCommunication, CookieIF* comCookie):
		DeviceHandlerBase(objectId, deviceCommunication, comCookie) {
	registers[0] = 0x00;
	registers[1] = 0x00;
	registers[2] = 0x00;
	registers[3] = 0x00;
	registers[4] = 0x00;

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

    case STATE_CHECK_REGISTERS:
        if (setupMGM() == RETURN_OK) {
            for (size_t i = 1; i <= MGMLIS3MDL::NR_OF_CTRL_REGISTERS; i++) {
                if (registers[i - 1] != commandBuffer[i]) {
                    break;
                }
            }
            setMode(_MODE_TO_ON);
        }

        break;

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
        *id = MGMLIS3MDL::READALL_MGM;
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

ReturnValue_t MGMHandlerLIS3MDL::setupMGM() {

	registers[0] = MGMLIS3MDL::CTRL_REG1_DEFAULT;
	registers[1] = MGMLIS3MDL::CTRL_REG2_DEFAULT;
	registers[2] = MGMLIS3MDL::CTRL_REG3_DEFAULT;
	registers[3] = MGMLIS3MDL::CTRL_REG4_DEFAULT;
	registers[4] = MGMLIS3MDL::CTRL_REG5_DEFAULT;

	return prepareRegisterWrite();

}

ReturnValue_t MGMHandlerLIS3MDL::buildNormalDeviceCommand(
		DeviceCommandId_t *id) {
    //defines CommandID of MGM in normal operation and build command from command
	*id = MGMLIS3MDL::READALL_MGM;
	return buildCommandFromCommand(*id, NULL, 0);
}

ReturnValue_t MGMHandlerLIS3MDL::buildCommandFromCommand(
        DeviceCommandId_t deviceCommand, const uint8_t *commandData,
		size_t commandDataLen) {
    lastSentCommand = deviceCommand;
    switch(deviceCommand) {
    case(MGMLIS3MDL::READALL_MGM): {
        std::memset(commandBuffer, 0, sizeof(commandBuffer));
        commandBuffer[0] = readCommand(0, true);

        rawPacket = commandBuffer;
        rawPacketLen = sizeof(commandBuffer);
        return RETURN_OK;
	case(MGMLIS3MDL::IDENTIFY_DEVICE): {
	    return identifyDevice();
	}
	case(MGMLIS3MDL::TEMP_SENSOR_ENABLE): {
	    return enableTemperatureSensor(commandData, commandDataLen);
	}
	case(MGMLIS3MDL::SETUP_MGM): {
	    return setupMGM();
	}
	case(MGMLIS3MDL::ACCURACY_OP_MODE_SET): {
	    return setOperatingMode(commandData, commandDataLen);
	}
	default:
	    lastSentCommand = DeviceHandlerIF::NO_COMMAND;
	    return DeviceHandlerIF::COMMAND_NOT_IMPLEMENTED;
	}
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
	if (len == MGMLIS3MDL::TOTAL_NR_OF_ADRESSES + 1) {
		*foundLen = len;
		*foundId = MGMLIS3MDL::READALL_MGM;
		//WHO AM I test
		if (*(start + 16) != MGMLIS3MDL::DEVICE_ID) {
			return DeviceHandlerIF::INVALID_DATA;
		}

	} else if (len == MGMLIS3MDL::SETUP_REPLY) {
		*foundLen = len;
		*foundId = MGMLIS3MDL::SETUP_MGM;
	} else if (len == SINGLE_COMMAND_ANSWER_LEN) {
		*foundLen = len;
		*foundId = lastSentCommand;
	} else {

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
	case MGMLIS3MDL::READALL_MGM: {
	    // TODO: Store configuration and sensor values in new local datasets.
		registers[0] = *(packet + 33);
		registers[1] = *(packet + 34);
		registers[2] = *(packet + 35);
		registers[3] = *(packet + 36);
		registers[4] = *(packet + 37);

		uint8_t scale = getFullScale(registers[2]);
		float sensitivityFactor = getSensitivityFactor(scale);

		uint8_t *accessBuffer;
		accessBuffer = const_cast<uint8_t*>(packet + 41);

		int16_t mgmMeasurementRawX = *(accessBuffer + 1) << 8 | *(accessBuffer);
		accessBuffer += 2;
		int16_t mgmMeasurementRawY = *(accessBuffer + 1) << 8 | *(accessBuffer);
		accessBuffer += 2;
		int16_t mgmMeasurementRawZ = *(accessBuffer + 1) << 8 | *(accessBuffer);
		accessBuffer += 2;

		int16_t tempValueRaw = *(accessBuffer + 1) << 8 | *(accessBuffer);

		// Target value in Gauss
		float mgmX = static_cast<float>(mgmMeasurementRawX) * sensitivityFactor;
		float mgmY = static_cast<float>(mgmMeasurementRawY) * sensitivityFactor;
		float mgmZ = static_cast<float>(mgmMeasurementRawZ) * sensitivityFactor;
		float tempValue = 25.0 + ((static_cast<float>(tempValueRaw)) / 8.0);

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

	return prepareRegisterWrite();
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
	insertInCommandAndReplyMap(MGMLIS3MDL::READALL_MGM, 1);
	insertInCommandAndReplyMap(MGMLIS3MDL::SETUP_MGM, 1);
	insertInCommandAndReplyMap(MGMLIS3MDL::IDENTIFY_DEVICE, 1);
	insertInCommandAndReplyMap(MGMLIS3MDL::TEMP_SENSOR_ENABLE, 1);
	insertInCommandAndReplyMap(MGMLIS3MDL::ACCURACY_OP_MODE_SET, 1);
}

ReturnValue_t MGMHandlerLIS3MDL::prepareRegisterWrite() {

	commandBuffer[0] = writeCommand(MGMLIS3MDL::CTRL_REG1, true);

	for (size_t i = 0; i < MGMLIS3MDL::NR_OF_CTRL_REGISTERS; i++) {
		commandBuffer[i + 1] = registers[i];
	}
	rawPacket = commandBuffer;
	rawPacketLen = MGMLIS3MDL::NR_OF_CTRL_REGISTERS;

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
