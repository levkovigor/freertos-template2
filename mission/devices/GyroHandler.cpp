#include <mission/devices/GyroHandler.h>

#if defined(at91sam9g20)
#include <sam9g20/comIF/cookies/I2cCookie.h>
#include <sam9g20/comIF/cookies/SpiCookie.h>
#endif

#include <cmath>
GyroHandler::GyroHandler(object_id_t objectId, object_id_t comIF,
        CookieIF *comCookie, uint8_t switchId):
        DeviceHandlerBase(objectId, comIF, comCookie), switchId(switchId) {
#if defined(at91sam9g20)
	if(dynamic_cast<SpiCookie*>(comCookie) != nullptr) {
		comInterface = CommInterface::SPI;
	}
	else if(dynamic_cast<I2cCookie*>(comCookie) != nullptr) {
		comInterface = CommInterface::I2C;
	}
#endif
}

GyroHandler::~GyroHandler() {
}

void GyroHandler::doStartUp() {
	switch(internalState) {
	case(InternalState::NONE): {
		internalState = InternalState::MODE_SELECT;
		break;
	}
	case(InternalState::MODE_SELECT): {
		if(commandExecuted) {
			internalState = InternalState::POWERUP;
			commandExecuted = false;
		}
		break;
	}
	case(InternalState::POWERUP): {
		if(commandExecuted) {
			internalState = InternalState::READ_PMU_STATUS;
			commandExecuted = false;
		}
		break;
	}
	case(InternalState::READ_PMU_STATUS): {
		if(commandExecuted) {
			internalState = InternalState::WRITE_RANGE;
			commandExecuted = false;
		}
		break;
	}
	case(InternalState::WRITE_RANGE): {
		if(commandExecuted) {
			internalState = InternalState::READ_RANGE;
			commandExecuted = false;
		}
		break;
	}
	case(InternalState::READ_RANGE): {
		if(commandExecuted) {
			internalState = InternalState::RUNNING;
			commandExecuted = false;
			setMode(MODE_NORMAL);
		}
		break;
	}
	default:
		break;
	}
}

void GyroHandler::doShutDown() {
}

ReturnValue_t GyroHandler::buildNormalDeviceCommand(DeviceCommandId_t *id) {
	switch(internalState) {
	case(InternalState::RUNNING): {
		// Poll Gyro Device. Perform block read of 7 bytes to read register
		// 0x12-0x17
		commandBuffer[0] = GYRO_DATA;
		std::memset(commandBuffer + 1, 0, 6);
		DeviceHandlerBase::rawPacket = commandBuffer;
		DeviceHandlerBase::rawPacketLen = 7;
		*id = GYRO_DATA;
		break;
	}
	default:
		break;
	}

	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t GyroHandler::buildTransitionDeviceCommand(DeviceCommandId_t *id) {
	// Check whether a reply is pending.
	if(isAwaitingReply()){
		return DeviceHandlerBase::NOTHING_TO_SEND;
	}

	if(internalState == InternalState::MODE_SELECT) {
		if(comInterface == CommInterface::SPI) {
			// Switch to SPI communication by writing to the 0x7F register
			*id = SPI_SELECT;
			commandBuffer[0] = SPI_MODE_SELECT;
			commandBuffer[1] = 0x00;
			DeviceHandlerBase::rawPacket = commandBuffer;
			DeviceHandlerBase::rawPacketLen = 2;
			return HasReturnvaluesIF::RETURN_OK;
		}
		else {
			// Proceed with power up immediately.
			internalState = InternalState::POWERUP;
		}
	}

	switch(internalState) {
	case(InternalState::MODE_SELECT): {
		break;
	}
	case(InternalState::POWERUP): {
		*id = WRITE_POWER;
		commandBuffer[sizeof(commandBuffer)] = POWER_CONFIG;
		return buildCommandFromCommand(*id,
				commandBuffer + sizeof(commandBuffer), 1);
	}
	case(InternalState::READ_PMU_STATUS): {
		*id = READ_PMU;
		return buildCommandFromCommand(*id, nullptr, 0);
	}
	case(InternalState::WRITE_RANGE): {
		*id = WRITE_RANGE;
		commandBuffer[sizeof(commandBuffer)] = RANGE_CONFIG;
		return buildCommandFromCommand(*id,
				commandBuffer + sizeof(commandBuffer), 1);
	}
	case(InternalState::READ_RANGE): {
		*id = READ_RANGE;
		return buildCommandFromCommand(*id, nullptr, 0);
	}
	default:
		sif::error << "GyroHandler::buildTransitionDeviceCommand: Invalid"
				"internal state!" << std::endl;
	}

	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t GyroHandler::buildCommandFromCommand(
        DeviceCommandId_t deviceCommand, const uint8_t *commandData,
		size_t commandDataLen) {
	switch(deviceCommand) {
	case(WRITE_POWER): {
		commandBuffer[0] = WRITE_POWER;
		commandBuffer[1] = commandData[0];
		DeviceHandlerBase::rawPacket = commandBuffer;
		DeviceHandlerBase::rawPacketLen = 2;
		break;
	}
	case(WRITE_RANGE): {
		commandBuffer[0] = WRITE_RANGE;
		commandBuffer[1] = commandData[0];
		DeviceHandlerBase::rawPacket = commandBuffer;
		DeviceHandlerBase::rawPacketLen = 2;
		break;
	}
	case(READ_RANGE): {
		commandBuffer[0] = READ_RANGE;
		commandBuffer[1] = 0x00;
		DeviceHandlerBase::rawPacket = commandBuffer;
		DeviceHandlerBase::rawPacketLen = 2;
		break;
	}
	case(READ_PMU): {
		commandBuffer[0] = READ_PMU;
		commandBuffer[1] = 0x00;
		DeviceHandlerBase::rawPacket = commandBuffer;
		DeviceHandlerBase::rawPacketLen = 2;
		break;
	}
	default:
		//Unknown DeviceCommand
		return DeviceHandlerIF::COMMAND_NOT_IMPLEMENTED;
	}
	return HasReturnvaluesIF::RETURN_OK;
}

void GyroHandler::fillCommandAndReplyMap() {
	this->insertInCommandAndReplyMap(WRITE_POWER, 2);
	this->insertInCommandAndReplyMap(READ_PMU, 8);
	this->insertInCommandAndReplyMap(WRITE_RANGE, 3);
	this->insertInCommandAndReplyMap(READ_RANGE, 3);
	this->insertInCommandAndReplyMap(SPI_SELECT, 3);
	this->insertInCommandAndReplyMap(GYRO_DATA, 3);
}

ReturnValue_t GyroHandler::scanForReply(const uint8_t *start,
        size_t remainingSize, DeviceCommandId_t *foundId, size_t *foundLen) {
	switch(internalState) {
	case(InternalState::MODE_SELECT): {
		commandExecuted = true;
		*foundId = SPI_SELECT;
		*foundLen = 2;
		break;
	}
	case(InternalState::POWERUP): {
		commandExecuted = true;
		*foundId = WRITE_POWER;
		*foundLen = 2;
		break;
	}
	case(InternalState::READ_PMU_STATUS): {
		*foundId = READ_PMU;
		*foundLen = 2;
		break;
	}
	case(InternalState::WRITE_RANGE): {
		commandExecuted = true;
		*foundId = WRITE_RANGE;
		*foundLen = 2;
		break;
	}
	case(InternalState::READ_RANGE): {
		*foundId = READ_RANGE;
		*foundLen = 2;
		break;
	}
	case(InternalState::RUNNING): {
		*foundId = GYRO_DATA;
		*foundLen = 7;
		break;
	default:
		return IGNORE_REPLY_DATA;
	}
	}
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t GyroHandler::interpretDeviceReply(DeviceCommandId_t id,
        const uint8_t *packet) {
	switch(internalState) {
	case(InternalState::READ_PMU_STATUS): {
		if (id == READ_PMU and packet[1] == 0b0000'0100) {
			// Bootup performed successfully.
			// PMU value should be stored somewhere

			if(mode == _MODE_START_UP) {
				commandExecuted = true;
			}
			else {
				internalState = InternalState::RUNNING;
			}
		}
		else {
			// do some error handling here.
			// try to send power command again.
			internalState = InternalState::POWERUP;
			// If this happens too often, the device will be power cycled.
			return HasReturnvaluesIF::RETURN_FAILED;
		}
		break;
	}
	case(InternalState::READ_RANGE): {
		if(id == READ_RANGE and packet[1] == RANGE_CONFIG) {
			// Range set successfully.
			// range should be stored somewhere.
			if(mode == _MODE_START_UP) {
				commandExecuted = true;
			}
			else {
				internalState = InternalState::RUNNING;
			}
		}
		else {
			// do some error handling here. Resend command
			// or restart
			internalState = InternalState::READ_RANGE;
			// If this happens too often, the device will be power cycled.
			return HasReturnvaluesIF::RETURN_FAILED;
		}
		break;
	}

	case(InternalState::RUNNING): {
		if(id == GYRO_DATA) {
			int16_t angularVelocityBinaryX = packet[2] << 8 | packet[1];
			int16_t angularVelocityBinaryY = packet[4] << 8 | packet[3];
			int16_t angularVelocityBinaryZ = packet[6] << 8 | packet[5];

			float angularVelocityX =
					angularVelocityBinaryX / std::pow(2, 15) * GYRO_RANGE;
			float angularVelocityY =
					angularVelocityBinaryY / std::pow(2, 15) * GYRO_RANGE;
			float angularVelocityZ =
					angularVelocityBinaryZ / std::pow(2, 15) * GYRO_RANGE;

			if(counter == 20) {
				sif::info << "GyroHandler: Angular velocities in degrees per "
						"second:" << std::endl;
				sif::info << "X: " << angularVelocityX << std::endl;
				sif::info << "Y: " << angularVelocityY << std::endl;
				sif::info << "Z: " << angularVelocityZ << std::endl;
				// Gyro values should be stored in pool.
				counter = 0;
			}
			else {
				counter ++;
			}

		}
		break;
	}
	default:
		break;
	}
    return HasReturnvaluesIF::RETURN_OK;
}

void GyroHandler::setNormalDatapoolEntriesInvalid() {
}

void GyroHandler::debugInterface(uint8_t positionTracker, object_id_t objectId,
        uint32_t parameter) {
}

uint32_t GyroHandler::getTransitionDelayMs(Mode_t modeFrom, Mode_t modeTo) {
    return 5000;
}

ReturnValue_t GyroHandler::getSwitches(const uint8_t **switches,
        uint8_t *numberOfSwitches) {
    return NO_SWITCH;
}

ReturnValue_t GyroHandler::initialize() {
    setMode(_MODE_START_UP);
    return DeviceHandlerBase::initialize();
}
