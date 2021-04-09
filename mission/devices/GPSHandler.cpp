#include "GPSHandler.h"

#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/serviceinterface/ServiceInterface.h>
#include <fsfw/devicehandlers/AcceptsDeviceResponsesIF.h>

GPSHandler::GPSHandler(object_id_t objectId_, object_id_t comIF_,
        CookieIF * cookie_, uint8_t powerSwitchId):
        DeviceHandlerBase(objectId_ ,comIF_, cookie_), switchId(powerSwitchId),
        commandExecuted(false), firstReplyReceived(false), navMessage() {
}

GPSHandler::~GPSHandler() {}

void GPSHandler::doStartUp() {
    switch (internalState) {
    case InternalState::STATE_NONE:
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "GPS Handler: Peforming start-up" << std::endl;
#else
        sif::printInfo("GPSHandler: Performing start-up\n");
#endif
        internalState = InternalState::STATE_WAIT_FIRST_MESSAGE;
        break;
    case InternalState::STATE_WAIT_FIRST_MESSAGE:
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "GPSHandler: Waiting for first message" << std::endl;
#else
        sif::printInfo("GPSHandler: Waiting for first message\n");
#endif
        if(firstReplyReceived) {
            internalState = InternalState::STATE_SET_UPDATE_RATE;
        }
        break;
    case InternalState::STATE_SET_UPDATE_RATE:
        if (commandExecuted) {
            internalState = InternalState::STATE_SET_MESSAGE_TYPE;
            commandExecuted = false;
        }
        break;
    case InternalState::STATE_SET_MESSAGE_TYPE: {
        if (commandExecuted) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << "GPS Handler: Configuration successfull! Switching On" << std::endl;
#else
            sif::printInfo("GPS Handler: Configuration successfull! Switching On\n");
#endif
            internalState = InternalState::STATE_CONFIGURED;
            // I set this to MODE_ON because we don't have a
            // TO_ON transition command yet
            setMode(MODE_ON);
        }
    }
    break;
    default: {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "GPSHandler: Unknown internal state!" << std::endl;
#else
        sif::printError("GPSHandler: Unknown internal state!\n");
#endif
        return;
    }
    }
}


void GPSHandler::doShutDown() {
    internalState = InternalState::STATE_NONE;
    commandExecuted = false;
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "GPS Device: Switching Off" << std::endl;
#else
    sif::printInfo("GPS Device: Switching Off\n");
#endif
    setMode(MODE_OFF);
    return;
}


ReturnValue_t GPSHandler::buildTransitionDeviceCommand(DeviceCommandId_t* id) {
    if(isAwaitingReply()){
        return DeviceHandlerBase::NOTHING_TO_SEND;
    }
    // Only startup commands are relevant for the GPS device
    switch(internalState) {
    case InternalState::STATE_NONE:
    case InternalState::STATE_WAIT_FIRST_MESSAGE:
    case InternalState::STATE_CONFIGURED:
        return DeviceHandlerBase::NOTHING_TO_SEND;
    case InternalState::STATE_SET_UPDATE_RATE: {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "GPS Handler: Building a SET_GPS_UPDATE_RATE command now" << std::endl;
#else
        sif::printInfo("GPS Handler: Building a SET_GPS_UPDATE_RATE command now\n");
#endif
        *id = GPSHandler::SET_GPS_UPDATE_RATE;
        uint8_t defaultStartupCommand[2];
        defaultStartupCommand[0] = baudRateSelect::RATE_9600;
        defaultStartupCommand[1] = attributes::SRAM;
        return buildCommandFromCommand(*id,defaultStartupCommand,2);
    }
    case InternalState::STATE_SET_MESSAGE_TYPE: {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "GPS Handler: Building a SET_GPS_MESSAGE_TYPE command now" << std::endl;
#else
        sif::printInfo("GPS Handler: Building a SET_GPS_MESSAGE_TYPE command now\n");
#endif
        *id = GPSHandler::SET_GPS_MESSAGE_TYPE;
        uint8_t defaultStartupCommand[2];
        defaultStartupCommand[0] = defaultMessageType;
        defaultStartupCommand[1] = attributes::SRAM;
        return buildCommandFromCommand(*id,defaultStartupCommand,2);
    }
    default:
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "GPS Handler: Invalid internal state" << std::endl;
#else
        sif::printError("GPS Handler: Invalid internal state\n");
#endif
        return HasReturnvaluesIF::RETURN_FAILED;
    }
}


ReturnValue_t GPSHandler::buildNormalDeviceCommand(DeviceCommandId_t* id) {
    // normal mode not used for GPS device
    return RETURN_OK;
}


ReturnValue_t GPSHandler::buildCommandFromCommand(
        DeviceCommandId_t deviceCommand, const uint8_t *commandData,
        size_t commandDataLen) {
    uint8_t result;
    // this value is adjusted to the real size in final command preparation
    uint8_t sizeOfCommandToBuild = BINARY_HEADER_SIZE;
    uint16_t payloadLength = 0;

    switch (deviceCommand) {
    case SET_GPS_UPDATE_RATE:
        payloadLength = 3;
        result = buildGpsUpdateRateCommand(commandData,commandDataLen);
        break;
    case SET_GPS_MESSAGE_TYPE:
        payloadLength = 3;
        result = buildMessageTypeSelectCommand(commandData,commandDataLen);
        break;
    case SET_GPS_BAUDRATE:
        payloadLength = 4;
        result = buildBaudSelectCommand(commandData,commandDataLen);
        break;
    case RESTART_DEVICE:
        // TODO: Implement building a restart command
        result = RETURN_OK;
        break;
    default:
        //Unknown DeviceCommand
        return DeviceHandlerIF::COMMAND_NOT_IMPLEMENTED;
    }

    prepareGpsCommandHeaderAndTail(deviceCommand, &sizeOfCommandToBuild,
            &payloadLength);

    DeviceHandlerBase::rawPacket = commandBuffer;
    DeviceHandlerBase::rawPacketLen = sizeOfCommandToBuild;
    return result;
}


ReturnValue_t GPSHandler::buildMessageTypeSelectCommand(
        const uint8_t *commandData, size_t commandDataLen) {

    if (commandDataLen != sizeof(GpsSetMessageType)) {
        return DeviceHandlerIF::INVALID_NUMBER_OR_LENGTH_OF_PARAMETERS;
    }
    const GpsSetMessageType *messageType =
            reinterpret_cast<const GpsSetMessageType*>(commandData);

    if (messageType->type > 2) {
        return DeviceHandlerIF::INVALID_COMMAND_PARAMETER;
    }
    commandBuffer[5] = messageType->type;
    if (messageType->attributes > 2) {
        return DeviceHandlerIF::INVALID_COMMAND_PARAMETER;
    }
    commandBuffer[6] = messageType->attributes;
    return RETURN_OK;
}


ReturnValue_t GPSHandler::buildBaudSelectCommand(const uint8_t *commandData,
        size_t commandDataLen) {
    if (commandDataLen != sizeof(GpsSetBaudRate)) {
        return DeviceHandlerIF::INVALID_NUMBER_OR_LENGTH_OF_PARAMETERS;
    }
    // casting the structure which is used for MIB export onto the
    // commandData to use it directly
    const GpsSetBaudRate *baudRate =
            reinterpret_cast<const GpsSetBaudRate*>(commandData);
    if(baudRate == nullptr) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    commandBuffer[5] = 0; // Comm Port is zero
    if (baudRate->baudRateSelect > 8) {
        return DeviceHandlerIF::INVALID_COMMAND_PARAMETER;
    }
    commandBuffer[6] = baudRate->baudRateSelect;
    if (baudRate->attributes > 3) {
        return DeviceHandlerIF::INVALID_COMMAND_PARAMETER;
    }
    commandBuffer[7] = baudRate->attributes;
    return RETURN_OK;
}


ReturnValue_t GPSHandler::buildGpsUpdateRateCommand(const uint8_t *commandData,
        size_t commandDataLen) {
    if (commandDataLen != sizeof(GpsSetUpdateRate)) {
        return DeviceHandlerIF::INVALID_NUMBER_OR_LENGTH_OF_PARAMETERS;
    }
    const GpsSetUpdateRate *updateRate =
            reinterpret_cast<const GpsSetUpdateRate*>(commandData);
    if(updateRate == nullptr) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }

    if (updateRate->updateRate > 50 || updateRate == 0) {
        return DeviceHandlerIF::INVALID_COMMAND_PARAMETER;
    }
    commandBuffer[5] = updateRate->updateRate;
    if (updateRate->attributes > 2) {
        return DeviceHandlerIF::INVALID_COMMAND_PARAMETER;
    }
    commandBuffer[6] = updateRate->attributes;
    return RETURN_OK;
}


void GPSHandler::prepareGpsCommandHeaderAndTail(DeviceCommandId_t deviceCommand,
        uint8_t * sizeOfCommandToBuild, uint16_t * payloadLength) {
    commandBuffer[0] = (START_OF_SEQUENCE & 0xFF00) >> 8;
    commandBuffer[1] = (START_OF_SEQUENCE & 0xFF);
    commandBuffer[2] = (*payloadLength & 0xFF00) >> 8;
    commandBuffer[3] = *payloadLength & 0xFF;
    commandBuffer[4] = deviceCommand;

    *sizeOfCommandToBuild += *payloadLength;
    commandBuffer[*sizeOfCommandToBuild] = calcChecksum(
            commandBuffer + BINARY_HEADER_SIZE, *payloadLength);
    *sizeOfCommandToBuild += 1;
    commandBuffer[*sizeOfCommandToBuild] = (END_OF_SEQUENCE & 0xFF00) >> 8;
    *sizeOfCommandToBuild += 1;
    commandBuffer[*sizeOfCommandToBuild] = (END_OF_SEQUENCE & 0xFF);
    *sizeOfCommandToBuild += 1;
}


void GPSHandler::fillCommandAndReplyMap() {
    insertInCommandAndReplyMap(SET_GPS_BAUDRATE,8);
    insertInCommandAndReplyMap(SET_GPS_NMEA_OUTPUT,8);
    insertInCommandAndReplyMap(SET_GPS_MESSAGE_TYPE,8);
    insertInCommandAndReplyMap(SET_GPS_UPDATE_RATE,8);
    insertInReplyMap(NAVIGATION_DATA_MESSAGE, 4, nullptr, true);
}


ReturnValue_t GPSHandler::scanForReply(const uint8_t *start, size_t len,
        DeviceCommandId_t *foundId, size_t *foundLen) {
    if(len < MIN_REPLY_LENGTH){
        return IGNORE_REPLY_DATA;
    }
    ReturnValue_t result;
    uint16_t startOfSequence;
    startOfSequence = start[0] << 8 | start[1];
    if(startOfSequence == START_OF_SEQUENCE) {
        result = checkBinaryReply(start,&len,foundId,foundLen);
        return result;
    }
    else if(startOfSequence == NMEA_START_OF_SEQUENCE){
        *foundId = NMEA_MESSAGE;
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::debug << "GPS Handler: Received NMEA string" << std::endl;
#else
        sif::printDebug("GPS Handler: Received NMEA string\n");
#endif
        // should we specify the size?
        // maybe we are going to use NMEA messages at some point ?
        uint8_t nmeaTypeBuffer[NMEA_TYPE_SIZE];
        memcpy(nmeaTypeBuffer, start + 2, NMEA_TYPE_SIZE);
        // assuming we dont use the NMEA packets for now, ignore the packet
        if(internalState == InternalState::STATE_WAIT_FIRST_MESSAGE) {
            firstReplyReceived = true;
        }
        return IGNORE_FULL_PACKET;
    }
    else {
        return IGNORE_REPLY_DATA;
    }
}


ReturnValue_t GPSHandler::checkBinaryReply(const uint8_t *start, size_t * len,
        DeviceCommandId_t *foundId, size_t *foundLen) {
    uint16_t payloadSize = start[2] << 8 | start[3];
    if((payloadSize + BINARY_HEADER_AND_TAIL_SIZE) > static_cast<uint16_t>(*len)) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "GPSHandler::checkBinaryReply: Invalid size !" << std::endl;
#else
        sif::printInfo("GPSHandler::checkBinaryReply: Invalid size!\n");
#endif
        return IGNORE_REPLY_DATA;
    }

    uint16_t size = 0;
    switch(start[PAYLOAD_START_INDEX]) {
    case NAVIGATION_DATA_MESSAGE:
        *foundId = NAVIGATION_DATA_MESSAGE;
        //sif::info << "GPS Handler: Got nav data " << std::hex <<
        //     *foundId << std::endl;
        size = navMessage.getSerializedSize();
        *foundLen = size;
        break;
    case NACK:
    case ACK:
        // assigns message type of corresponding command
        *foundId = start[PAYLOAD_START_INDEX + 1];
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "GPSHandler: Got reply " << std::hex << *foundId << std::endl;
#else
        sif::printInfo("GPSHandler: Got reply 0x%08x", *foundId);
#endif
        size = 2 + BINARY_HEADER_AND_TAIL_SIZE;
        break;
    default:
        * foundLen = 1;
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "GPSHandler::checkBinaryReply: Ignoring packet because start ID is weird"
                << std::endl;
#else
        sif::printInfo("GPSHandler::checkBinaryReply: Ignoring packet because start ID is weird\n");
#endif
        return IGNORE_FULL_PACKET;
    }

    uint8_t foundChecksum = start[size-3];
    uint8_t calculatedChecksum = calcChecksum(start + BINARY_HEADER_SIZE,
            payloadSize);
    if(foundChecksum != calculatedChecksum) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "GPSHandler: Checksum error!" << std::endl;
#else
        sif::printError("GPSHandler: Checksum error!\n");
#endif
        return DeviceHandlerIF::INVALID_DATA;
    }
    *foundLen = size;
    if(*foundId == NAVIGATION_DATA_MESSAGE) {
        return DeviceHandlerBase::APERIODIC_REPLY;
    }
    return RETURN_OK;
}


ReturnValue_t GPSHandler::interpretDeviceReply(DeviceCommandId_t id,
        const uint8_t *packet) {

    switch (id) {
    case NAVIGATION_DATA_MESSAGE: {
        return interpretNavigationData(packet);
    }
    case SET_GPS_UPDATE_RATE:
    case SET_GPS_MESSAGE_TYPE:
    case SET_GPS_BAUDRATE: {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::debug << "GPS Handler: Binary reply received" << std::endl;
#else
        sif::printDebug("GPS Handler: Binary reply received\n");
#endif
        if(internalState == InternalState::STATE_WAIT_FIRST_MESSAGE) {
            firstReplyReceived = true;
        }
        if (packet[PAYLOAD_START_INDEX] == ACK) {
            commandExecuted = true;
            return HasReturnvaluesIF::RETURN_OK;
        }
        else {
            commandExecuted = false;
            return DEVICE_DID_NOT_EXECUTE;
        }
    }
    case NMEA_MESSAGE:
        return RETURN_OK;
    default:
        return DeviceHandlerIF::DEVICE_REPLY_INVALID;
    }
}

ReturnValue_t GPSHandler::interpretNavigationData(const uint8_t *packet) {
    //	sif::info << "GPS Handler: GPS packet received successfully !" << std::endl;
    //	if(internalState == InternalState::STATE_WAIT_FIRST_MESSAGE) {
    //		firstReplyReceived = true;
    //	}
    //	//ReturnValue_t result = navData.read();
    //	if(result != RETURN_OK) {
    //		sif::debug << "GPSHandler::interpretNavigationData: Could not read"
    //				" dataset with error code " << result << std::endl;
    //		// reset the dataset.
    //		//ReturnValue_t commitResult = navData.commit();
    //		if(commitResult != RETURN_OK) {
    //			sif::error << "GPSHandler::interpretNavigationData: Error "
    //					" committing faulty dataset!" << std::endl;
    //		}
    //		return result;
    //	}
    //	size_t size = navMessage.getSerializedSize();
    //
    //	result = navMessage.deSerialize(&packet, &size,
    //	        SerializeIF::Endianness::BIG);
    //	if(result != RETURN_OK) {
    //		return result;
    //	}
    //	//deSerializeNavigationDataIntoDataset(packet);
    //	checkAndStoreStructDataIntoDatapool();
    //	//result = navData.commit();
    //	return result;
    return HasReturnvaluesIF::RETURN_OK;
}

//void GPSHandler::deSerializeNavigationDataIntoDataset(const uint8_t * packet) {
//	// we can access data with our GPS struct by casting it on the packet
//	const GpsNavigationDataMessage * gpsData = reinterpret_cast<const GpsNavigationDataMessage *>(packet);
//	// removing constness to correct endianness for data analysis (ARM = little endian !)
//	endianCorrector.correctGpsDataEndianness(const_cast<GpsNavigationDataMessage *>(gpsData));
//	checkAndStoreStructDataIntoDatapool(gpsData);
//
//}

//void GPSHandler::checkAndStoreStructDataIntoDatapool() {
//	if (navMessage.fixMode != navData.fixMode.value) {
//		if ((navMessage.fixMode> 2) && (navData.fixMode.value <2)) {
//			triggerEvent(GPS_FIX, navMessage.fixMode);
//		} else if (navMessage.fixMode == 0) {
//			triggerEvent(GPS_LOST_FIX, navMessage.fixMode);
//		}
//		navData.fixMode = navMessage.fixMode;
//	}
//
//	if(navMessage.fixMode < 2) {
//		//navData.velocityECEF.setValid(PoolVariableIF::INVALID);
//		navData.setEntriesValid(PoolVariableIF::INVALID);
//	}
//	else {
//		navData.setEntriesValid(PoolVariableIF::VALID);
//	}
//	navData.numberOfSvInFix = navMessage.numberOfSvInFix;
//
//	navData.velocityECEF.value[0] = navMessage.ecefVx;
//	navData.velocityECEF.value[1] = navMessage.ecefVy;
//	navData.velocityECEF.value[2] = navMessage.ecefVz;
//	navData.positionECEF.value[0] = navMessage.ecefX;
//	navData.positionECEF.value[1] = navMessage.ecefY;
//	navData.positionECEF.value[2] = navMessage.ecefZ;
//
//	// we should check for value jumps here, but we need a threshold
//	// or is this done by a monitor class?
//	navData.latitude =  navMessage.latitude;
//	// sif::info << "GPS Handler: The latitude value as hex is: "
//	//      <<std::hex << gpsData->latitude << std::endl;
//	navData.longitude = navMessage.latitude;
//
//	navData.meanSeaLevelAltitude = navMessage.meanSeaLevelAltitude;
//	navData.gnssWeek = navMessage.gnssWeek;
//	navData.timeOfWeek = navMessage.timeOfWeek;
//}


void GPSHandler::setNormalDatapoolEntriesInvalid() {
    // Normal mode not used
}


uint32_t GPSHandler::getTransitionDelayMs(Mode_t modeFrom, Mode_t modeTo) {
    if(modeFrom == _MODE_START_UP && modeTo == MODE_ON)
    {
        return STARTUP_TIMEOUT_MS;
    }
    else if (modeFrom == MODE_ON && modeTo == _MODE_POWER_DOWN)
    {
        return SHUTDOWN_TIMEOUT_MS;
    }
    else
    {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::debug << "GPS Handler: This timeout has not been set" << std::endl;
        sif::debug << "Mode from: " << modeFrom << " Mode to: " << modeTo << std::endl;
#else
        sif::printDebug("GPS Handler: This timeout has not been set\n");
        sif::printDebug("Mode from: %lu | Mode to: %lu", static_cast<unsigned int>(modeFrom),
                static_cast<unsigned int>(modeTo));
#endif
        return 0;
    }
}


uint8_t GPSHandler::calcChecksum(const uint8_t * payload,uint16_t payloadSize) {
    uint8_t checksum = 0;
    for(int i = 0; i < payloadSize; i++) {
        checksum = checksum ^ payload[i];
    }
    return checksum;
}


// Remove this once (virtual) switch has been set up !!!
// this specifies that there is no switch currently implemented
ReturnValue_t GPSHandler::getSwitches(const uint8_t **switches,
        uint8_t *numberOfSwitches) {
    return NO_SWITCH;
}


