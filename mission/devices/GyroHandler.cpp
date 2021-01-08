#include "GyroHandler.h"
#include "devicedefinitions/GyroPackets.h"

#if defined(at91sam9g20)
#include <sam9g20/core/CoreController.h>
#endif

#if OBSW_ENHANCED_PRINTOUT == 1
#include <fsfw/globalfunctions/PeriodicOperationDivider.h>
#endif

#include <cmath>

GyroHandler::GyroHandler(object_id_t objectId, object_id_t comIF,
        CookieIF *comCookie, uint8_t switchId):
        DeviceHandlerBase(objectId, comIF, comCookie), switchId(switchId),
		gyroData(this), gyroConfigSet(this),
		selfTestDivider(5) {
#if OBSW_ENHANCED_PRINTOUT == 1
    debugDivider = new PeriodicOperationDivider(20);
#endif
    }

GyroHandler::~GyroHandler() {
}

void GyroHandler::doStartUp() {

	switch(internalState) {
	case(InternalStates::NONE): {
		internalState = InternalStates::MODE_SELECT;
		break;
	}
	case(InternalStates::MODE_SELECT): {
		if(commandExecuted) {
			internalState = InternalStates::WRITE_POWER;
			commandExecuted = false;
		}
		break;
	}
	case(InternalStates::WRITE_POWER): {
		if(commandExecuted) {
			internalState = InternalStates::READ_PMU_STATUS;
			commandExecuted = false;
		}
		break;
	}
	case(InternalStates::READ_PMU_STATUS): {
		if(commandExecuted) {
			internalState = InternalStates::WRITE_CONFIG;
			commandExecuted = false;
		}
		break;
	}
	case(InternalStates::WRITE_CONFIG): {
	    if(commandExecuted) {
	        internalState = InternalStates::READ_CONFIG;
	        commandExecuted = false;
	    }
	    break;
	}
	case(InternalStates::READ_CONFIG): {
	    if(commandExecuted) {
	        internalState = InternalStates::PERFORM_SELFTEST;
	        commandExecuted = false;
	    }
	    break;
	}
	// todo: make self-test optional via parameter?
	case(InternalStates::PERFORM_SELFTEST): {
	    if(commandExecuted) {
	        internalState = InternalStates::READ_STATUS;
	        commandExecuted = false;
	    }
	    break;
	}
	case(InternalStates::READ_STATUS): {
	    if(commandExecuted) {
	        internalState = InternalStates::RUNNING;
	        commandExecuted = true;
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
    case(InternalStates::RUNNING): {

#if defined(at91sam9g20)
        ReturnValue_t result = checkSelfTest(id);
        if(result != HasReturnvaluesIF::RETURN_FAILED) {
            return result;
        }
#endif

        // Poll Gyro Device. Perform block read of 7 bytes to read register
        // 0x12-0x17
        commandBuffer[0] = GyroDefinitions::GYRO_DATA_CMD;
        std::memset(commandBuffer + 1, 0, 6);
        DeviceHandlerBase::rawPacket = commandBuffer;
        DeviceHandlerBase::rawPacketLen = 7;
        *id = GyroDefinitions::GYRO_DATA;
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

	if(internalState == InternalStates::MODE_SELECT) {
		if(comInterface == CommInterface::SPI) {
			// Switch to SPI communication by writing to the 0x7F register
			*id = GyroDefinitions::SPI_SELECT;
			commandBuffer[0] = GyroDefinitions::SPI_MODE_SELECT;
			commandBuffer[1] = 0x00;
			DeviceHandlerBase::rawPacket = commandBuffer;
			DeviceHandlerBase::rawPacketLen = 2;
			return HasReturnvaluesIF::RETURN_OK;
		}
		else {
			// Proceed with power up immediately.
			internalState = InternalStates::WRITE_POWER;
		}
	}

	switch(internalState) {
	case(InternalStates::MODE_SELECT): {
		break;
	}
	case(InternalStates::WRITE_POWER): {
		*id = GyroDefinitions::WRITE_POWER;
		commandBuffer[sizeof(commandBuffer)] = GyroDefinitions::POWER_CONFIG;
		return buildCommandFromCommand(*id,
				commandBuffer + sizeof(commandBuffer), 1);
	}
	case(InternalStates::READ_PMU_STATUS): {
		*id = GyroDefinitions::READ_PMU;
		return buildCommandFromCommand(*id, nullptr, 0);
	}
	case(InternalStates::WRITE_CONFIG): {
	    *id = GyroDefinitions::WRITE_CONFIG;
	    gyroConfiguration[0] = GyroDefinitions::GYRO_DEF_CONFIG;
	    gyroConfiguration[1] = GyroDefinitions::RANGE_CONFIG;
	    return buildCommandFromCommand(*id, gyroConfiguration, 2);
	}
	case(InternalStates::READ_CONFIG): {
	    *id = GyroDefinitions::READ_CONFIG;
	    return buildCommandFromCommand(*id, nullptr, 0);
	}
	case(InternalStates::PERFORM_SELFTEST): {
	    *id = GyroDefinitions::PERFORM_SELFTEST;
	    return buildCommandFromCommand(*id, nullptr, 0);
	}
	case(InternalStates::READ_STATUS): {
	    *id = GyroDefinitions::READ_STATUS;
	    return buildCommandFromCommand(*id, nullptr, 0);
	}
	default:
		sif::error << "GyroHandler::buildTransitionDeviceCommand: Invalid"
				" internal state!" << std::endl;
	}

	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t GyroHandler::buildCommandFromCommand(
        DeviceCommandId_t deviceCommand, const uint8_t *commandData,
		size_t commandDataLen) {
	switch(deviceCommand) {
	case(GyroDefinitions::WRITE_POWER): {
		commandBuffer[0] = GyroDefinitions::POWER_REGISTER;
		commandBuffer[1] = commandData[0];
		DeviceHandlerBase::rawPacket = commandBuffer;
		DeviceHandlerBase::rawPacketLen = 2;
		if(mode == MODE_NORMAL) {
		    // external command, mode change necessary to identify reply.
		    internalState = InternalStates::WRITE_POWER;
		}
		break;
	}
	case(GyroDefinitions::WRITE_CONFIG): {
	    commandBuffer[0] = GyroDefinitions::CONFIG_REGISTER;
	    if(commandDataLen == 1) {
	        // Configuration written to 0x42, see p.35 of datasheet.
	        // Configuration is cached inside member.
	        gyroConfiguration[0] = commandData[0];
	        commandBuffer[1] = gyroConfiguration[0];
	        DeviceHandlerBase::rawPacketLen = 2;
	        break;
	    }
	    else if(commandDataLen == 2) {
	        // Configuration written to 0x43, see p.36 of datasheet.
	        // Configuration is cached inside member.
	        gyroConfiguration[0] = commandData[0];
	        gyroConfiguration[1] = commandData[1];
	        commandBuffer[1] = gyroConfiguration[0];
	        commandBuffer[2] = gyroConfiguration[1];
	        DeviceHandlerBase::rawPacketLen = 3;
	    }
	    else {
	        DeviceHandlerBase::rawPacketLen = 0;
	    }
	    if(mode == MODE_NORMAL) {
	        // external command, mode change necessary to identify reply.
	        internalState = InternalStates::WRITE_CONFIG;
	    }
        DeviceHandlerBase::rawPacket = commandBuffer;
        break;
    }
	case(GyroDefinitions::READ_CONFIG): {
	    commandBuffer[0] = GyroDefinitions::READ_CONFIG_CMD;
	    commandBuffer[1] = 0x00;
	    commandBuffer[2] = 0x00;
	    if(mode == MODE_NORMAL) {
	        // external command, mode change necessary to identify reply.
	        internalState = InternalStates::READ_CONFIG;
	    }
	    DeviceHandlerBase::rawPacketLen = 3;
	    DeviceHandlerBase::rawPacket = commandBuffer;
	    break;
	}
	case(GyroDefinitions::WRITE_RANGE): {
		commandBuffer[0] = GyroDefinitions::WRITE_RANGE;
		commandBuffer[1] = commandData[0];
		gyroConfiguration[1] = commandData[0];
		if(mode == MODE_NORMAL) {
		    // external command, mode change necessary to identify reply.
		    internalState = InternalStates::WRITE_RANGE;
		}
		DeviceHandlerBase::rawPacket = commandBuffer;
		DeviceHandlerBase::rawPacketLen = 2;
		break;
	}
	case(GyroDefinitions::READ_PMU): {
		commandBuffer[0] = GyroDefinitions::READ_PMU;
		commandBuffer[1] = 0x00;
		DeviceHandlerBase::rawPacket = commandBuffer;
		DeviceHandlerBase::rawPacketLen = 2;
		break;
	}

	case(GyroDefinitions::PERFORM_SELFTEST): {
	    commandBuffer[0] = GyroDefinitions::SELFTEST_REGISTER;
	    commandBuffer[1] = GyroDefinitions::PERFORM_SELFTEST_CMD;
	    DeviceHandlerBase::rawPacket = commandBuffer;
	    DeviceHandlerBase::rawPacketLen = 2;
	    break;
	}
	case(GyroDefinitions::READ_STATUS): {
	    commandBuffer[0] = GyroDefinitions::READ_STATUS_CMD;
	    commandBuffer[1] = 0x00;
	    if(mode == MODE_NORMAL) {
	        // external command, mode change necessary to identify reply.
	        internalState = InternalStates::READ_STATUS;
	    }
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
	this->insertInCommandAndReplyMap(GyroDefinitions::WRITE_POWER, 2);
	this->insertInCommandAndReplyMap(GyroDefinitions::READ_PMU, 8);
	this->insertInCommandAndReplyMap(GyroDefinitions::WRITE_RANGE, 3);
	this->insertInCommandAndReplyMap(GyroDefinitions::WRITE_CONFIG, 3);
	this->insertInCommandAndReplyMap(GyroDefinitions::READ_CONFIG, 3,
	        &gyroConfigSet);
	this->insertInCommandAndReplyMap(GyroDefinitions::SPI_SELECT, 3);
	this->insertInCommandAndReplyMap(GyroDefinitions::GYRO_DATA, 3, &gyroData);
	this->insertInCommandAndReplyMap(GyroDefinitions::PERFORM_SELFTEST, 3);
	this->insertInCommandAndReplyMap(GyroDefinitions::READ_STATUS, 3);
}

ReturnValue_t GyroHandler::scanForReply(const uint8_t *start,
        size_t remainingSize, DeviceCommandId_t *foundId, size_t *foundLen) {
	switch(internalState) {
	case(InternalStates::MODE_SELECT): {
	    if(mode == _MODE_START_UP) {
	        commandExecuted = true;
	    }
		*foundId = GyroDefinitions::SPI_SELECT;
		*foundLen = 2;
		break;
	}
	case(InternalStates::WRITE_POWER): {
	    if(mode == _MODE_START_UP) {
	        commandExecuted = true;
	    }
	    else {
	        // acknowledge reply and go back to normal mode immediately.
	        internalState = InternalStates::RUNNING;
	    }
		*foundId = GyroDefinitions::WRITE_POWER;
		*foundLen = 2;
		break;
	}
	case(InternalStates::READ_PMU_STATUS): {
		*foundId = GyroDefinitions::READ_PMU;
		*foundLen = 2;
		break;
	}
	case(InternalStates::WRITE_CONFIG): {
	    if(mode == _MODE_START_UP) {
	        commandExecuted = true;
	    }
	    else {
	        // acknowledge reply and go back to normal mode immediately.
	        internalState = InternalStates::RUNNING;
	    }
		*foundId = GyroDefinitions::WRITE_CONFIG;
		*foundLen = 3;
		break;
	}
	case(InternalStates::READ_CONFIG): {
		*foundId = GyroDefinitions::READ_CONFIG;
		*foundLen = 3;
		break;
	}
	case(InternalStates::WRITE_RANGE): {
	    internalState = InternalStates::RUNNING;
	    *foundId = GyroDefinitions::WRITE_RANGE;
	    *foundLen = 2;
	    break;
	}
	case(InternalStates::PERFORM_SELFTEST): {
	    if(mode == _MODE_START_UP) {
	        commandExecuted = true;
	    }
	    else {
	        internalState = InternalStates::RUNNING;
	    }
	    *foundId = GyroDefinitions::PERFORM_SELFTEST;
	    *foundLen = 2;
	    break;
	}
	case(InternalStates::READ_STATUS): {
	    *foundId = GyroDefinitions::READ_STATUS;
	    *foundLen = 2;
	    break;
	}
	case(InternalStates::RUNNING): {
		*foundId = GyroDefinitions::GYRO_DATA;
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

	case(InternalStates::READ_PMU_STATUS): {
	    if(packet[1] == 0xff) {
	        return HasReturnvaluesIF::RETURN_FAILED;
	    }
		if (id == GyroDefinitions::READ_PMU and packet[1] == 0b0000'0100) {
			// Bootup performed successfully.
			// PMU value should be stored somewhere

			if(mode == _MODE_START_UP) {
				commandExecuted = true;
			}
			else {
				internalState = InternalStates::RUNNING;
			}
		}
		else {
			// do some error handling here.
			// try to send power command again.
			internalState = InternalStates::WRITE_POWER;
			// If this happens too often, the device will be power cycled.
			return HasReturnvaluesIF::RETURN_FAILED;
		}
		break;
	}

	case(InternalStates::READ_CONFIG): {
	    if(packet[1] == 0xff or packet[2] == 0xff) {
	        return HasReturnvaluesIF::RETURN_FAILED;
	    }
	    // It is assumed that the configuration was cached inside the
	    // last two bytes of the command buffer, so the configuration
	    // must be equal to those parameters.
	    if((packet[1] == gyroConfiguration[0]) and
	            (packet[2] == gyroConfiguration[1])) {
	        // store values to dataset.
	    	ReturnValue_t result = gyroConfigSet.read();
	    	if(result != HasReturnvaluesIF::RETURN_OK) {
	    		return result;
	    	}

	    	if(not gyroConfigSet.isValid()) {
	    		gyroConfigSet.setValidity(true, true);
	    	}

	    	gyroConfigSet.gyroGeneralConfigReg42 = gyroConfiguration[0];
	    	gyroConfigSet.gyroRangeConfigReg43 = gyroConfiguration[1];

	    	result = gyroConfigSet.commit();
	    	if(result != HasReturnvaluesIF::RETURN_OK) {
	    		return result;
	    	}

	        if(mode == _MODE_START_UP) {
	            // Default configuration successfull.
	            commandExecuted = true;
	        }
	        else if(mode == MODE_NORMAL) {
	            // Was an external command, go back to polling mode.
	            internalState = InternalStates::RUNNING;
	        }
	    }
	    break;
	}

	case(InternalStates::READ_STATUS): {
	    if(packet[1] == 0xff) {
	        return HasReturnvaluesIF::RETURN_FAILED;
	    }
	    // store status to dataset.

	    // it is assumed that only the self-test register is read for now!
	    if((packet[1] & GyroDefinitions::SELFTEST_OK) == GyroDefinitions::SELFTEST_OK) {
	        if(mode == _MODE_START_UP) {
	            commandExecuted = true;
	        }
	        else {
	            selfTestFailCounter = 0;
	            checkSelfTestRegister = false;
	            internalState = InternalStates::RUNNING;
	        }
	    }
	    else {
	        if(mode == _MODE_START_UP) {
	            // startup timeout will be triggered.
	            return HasReturnvaluesIF::RETURN_OK;
	        }
	        if(selfTestFailCounter == 3) {
	            // Reboot Gyro here.
	            return HasReturnvaluesIF::RETURN_FAILED;
	        }
	        else {
	            selfTestFailCounter ++;
	            // Prevents the delay counter from being reset.
	            return IGNORE_REPLY_DATA;
	        }
	    }
	    break;
	}

	// Core method: Analyse received sensor data and store it into datapool.
	case(InternalStates::RUNNING): {
		if(id == GyroDefinitions::GYRO_DATA) {
			int16_t angularVelocityBinaryX = packet[2] << 8 | packet[1];
			int16_t angularVelocityBinaryY = packet[4] << 8 | packet[3];
			int16_t angularVelocityBinaryZ = packet[6] << 8 | packet[5];

			float scaleFactor = GYRO_RANGE / static_cast<float>(INT16_MAX);
			float angularVelocityX = angularVelocityBinaryX * scaleFactor;
			float angularVelocityY = angularVelocityBinaryY * scaleFactor;
			float angularVelocityZ = angularVelocityBinaryZ * scaleFactor;

#if OBSW_ENHANCED_PRINTOUT == 1
			if(debugDivider->checkAndIncrement()) {
				sif::info << "GyroHandler: Angular velocities in degrees per "
						"second:" << std::endl;
				sif::info << "X: " << angularVelocityX << std::endl;
				sif::info << "Y: " << angularVelocityY << std::endl;
				sif::info << "Z: " << angularVelocityZ << std::endl;
			}
#endif
			ReturnValue_t result = gyroData.read();
			if(result != HasReturnvaluesIF::RETURN_OK) {
				// Configuration error.
				return result;
			}

			if(not gyroData.isValid()) {
				gyroData.setValidity(true, true);
			}

			gyroData.angVelocityX = angularVelocityX;
			gyroData.angVelocityY = angularVelocityY;
			gyroData.angVelocityZ = angularVelocityZ;

			gyroData.commit();
			if(result != HasReturnvaluesIF::RETURN_OK) {
				// Configuration error.
				return result;
			}
		}
		break;
	}
	default:
		break;
	}
    return HasReturnvaluesIF::RETURN_OK;
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

void GyroHandler::modeChanged() {
	// TODO: test whether this works without the if-clause. It should.
	if(mode != MODE_NORMAL) {
		internalState = InternalStates::NONE;
	}
}

ReturnValue_t GyroHandler::initializeLocalDataPool(
		LocalDataPool &localDataPoolMap, LocalDataPoolManager &poolManager) {
	localDataPoolMap.emplace(GyroDefinitions::ANGULAR_VELOCITY_X,
			new PoolEntry<float>({0.0}));
	localDataPoolMap.emplace(GyroDefinitions::ANGULAR_VELOCITY_Y,
			new PoolEntry<float>({0.0}));
	localDataPoolMap.emplace(GyroDefinitions::ANGULAR_VELOCITY_Z,
			new PoolEntry<float>({0.0}));
	localDataPoolMap.emplace(GyroDefinitions::GENERAL_CONFIG_REG42,
			new PoolEntry<uint8_t>({0}));
	localDataPoolMap.emplace(GyroDefinitions::RANGE_CONFIG_REG43,
			new PoolEntry<uint8_t>({0}));

	poolManager.subscribeForPeriodicPacket(gyroData.getSid(), false, 4.0, false);
	return HasReturnvaluesIF::RETURN_OK;
}

#if defined(at91sam9g20)
ReturnValue_t GyroHandler::checkSelfTest(DeviceCommandId_t* id) {
    CoreController* coreController = objectManager->
            get<CoreController>(objects::CORE_CONTROLLER);
    if(coreController == nullptr) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    // TODO: This will not work.. Use core controller second counter
    // instead.
    uint32_t currentSecondUptime = coreController->getUptimeSeconds();
    // todo: make self-test optional via parameter.
    // perform self-test every week
    if((currentSecondUptime - lastSelfTestSeconds) >=
            SELF_TEST_PERIOD_SECOND) {
        *id = GyroDefinitions::PERFORM_SELFTEST;
        internalState = InternalStates::PERFORM_SELFTEST;
        checkSelfTestRegister = true;
        lastSelfTestSeconds = currentSecondUptime;
        return buildCommandFromCommand(*id, nullptr, 0);
    }

    // if a self-test check is pending, check every few cycles.
    if(checkSelfTestRegister) {
        if(selfTestDivider.checkAndIncrement()) {
            *id = GyroDefinitions::READ_STATUS;
            internalState = InternalStates::READ_STATUS;
            return buildCommandFromCommand(*id, nullptr, 0);
        }
    }
    return HasReturnvaluesIF::RETURN_FAILED;
}
#endif
