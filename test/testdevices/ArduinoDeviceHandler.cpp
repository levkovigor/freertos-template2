#include "ArduinoDeviceHandler.h"
#include <fsfw/serviceinterface/ServiceInterface.h>


ArduinoHandler::ArduinoHandler(object_id_t objectId, object_id_t comIF,
		CookieIF * cookie, std::string idString_):
        DeviceHandlerBase(objectId, comIF, cookie),
        idString(idString_) {
}

ArduinoHandler::~ArduinoHandler() {}

void ArduinoHandler::performOperationHook() {}

void ArduinoHandler::doStartUp() {
    setMode(MODE_ON);
}

void ArduinoHandler::doShutDown() {
    setMode(MODE_OFF);
}

ReturnValue_t ArduinoHandler::buildNormalDeviceCommand(DeviceCommandId_t *id) {
    // some simple command which is sent regularly
	if(awesomeMapIter == ArduinoHandler::awesomeMap.end()) {
		awesomeMapIter = ArduinoHandler::awesomeMap.begin();
	}
	std::string sendString = awesomeMapIter->second;

    if ( commandSendCounter == commandSendInterval ) {
    	*id = awesomeMapIter->first;
    	lastCommand = awesomeMapIter->first;
    	sendString = awesomeMapIter->second;

    	rawPacketLen = sendString.size();
    	sendData.reserve(rawPacketLen + 1);
    	std::copy(sendString.begin(), sendString.end(), sendData.data());
    	sendData[rawPacketLen] = '\0';
        rawPacket = sendData.data();
        commandSendCounter = 1;
        if(commandPrintCounter == commandPrintInterval) {
            printCommand(rawPacket, rawPacketLen);
        	commandPrintCounter = 1;
        }
        else {
        	commandPrintCounter++;
        }
        awesomeMapIter++;
        return RETURN_OK;
    } else {
        commandSendCounter++;
        return NOTHING_TO_SEND;
    }
}

void ArduinoHandler::printCommand(uint8_t* command, size_t command_size) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Arduino Handler sent to " << idString << ": " << command << std::endl;
#else
    sif::printInfo("Arduino Handler sent to %s: %s", idString.c_str(), command);
#endif
}

ReturnValue_t ArduinoHandler::buildTransitionDeviceCommand(
        DeviceCommandId_t *id) {
    return RETURN_OK;
}

ReturnValue_t ArduinoHandler::buildCommandFromCommand(
		DeviceCommandId_t deviceCommand, const uint8_t *commandData,
		size_t commandDataLen) {
	return RETURN_OK;
}

void ArduinoHandler::fillCommandAndReplyMap() {
	// The test string is echoed back. +1 for null terminator.
	for(awesomeMapIter = ArduinoHandler::awesomeMap.begin();
			awesomeMapIter != awesomeMap.end(); awesomeMapIter ++) {
		insertInCommandAndReplyMap(awesomeMapIter->first,3,
				nullptr, awesomeMapIter->second.size());
	}
}

ReturnValue_t ArduinoHandler::scanForReply(const uint8_t *start,
        size_t remainingSize, DeviceCommandId_t *foundId, size_t *foundLen) {
	size_t expectedSize = ArduinoHandler::awesomeMap[lastCommand].size();
	if(remainingSize == expectedSize) {
	    std::vector<char> readString(remainingSize);
		//char readString[remainingSize];
		memcpy(readString.data(), start, remainingSize);
		readString[remainingSize] = '\0';
		if(commandPrintCounter == commandPrintInterval) {
			printReply((unsigned char*)readString.data(), remainingSize);
			commandPrintCounter = 1;
		}
		else {
			commandPrintCounter++;
		}
		*foundId = lastCommand;
		*foundLen = remainingSize;
	}
	else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "Arduino Handler: Invalid Reply" << std::endl;
#else
		sif::printError("Arduino Handler: Invalid Reply\n");
#endif
	}
	return RETURN_OK;
}

void ArduinoHandler::printReply(uint8_t * reply, size_t reply_size) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Arduino Handler received echo reply from " << idString << ": " << reply
            << std::endl;
#else
    sif::printInfo("Arduino Handler received echo reply from %s: %s\n", idString.c_str(), reply);
#endif
}

ReturnValue_t ArduinoHandler::interpretDeviceReply(DeviceCommandId_t id,
        const uint8_t *packet) {
    return RETURN_OK;
}

void ArduinoHandler::setNormalDatapoolEntriesInvalid() {
}

uint32_t ArduinoHandler::getTransitionDelayMs(Mode_t modeFrom, Mode_t modeTo) {
	return 2000;
}

// Set this in config !
ReturnValue_t ArduinoHandler::initialize() {
    awesomeMap.emplace(TEST_COMMAND_0, string0);
    awesomeMap.emplace(TEST_COMMAND_1, string1);
    awesomeMap.emplace(TEST_COMMAND_2, string2);
    awesomeMap.emplace(TEST_COMMAND_3, string3);
    awesomeMapIter = ArduinoHandler::awesomeMap.begin();

	ReturnValue_t result = DeviceHandlerBase::initialize();
	if(result == RETURN_OK) {
	    mode = _MODE_START_UP;
	}
	return result;
}

ReturnValue_t ArduinoHandler::getSwitches(const uint8_t **switches,
        uint8_t *numberOfSwitches) {
    return NO_SWITCH;
}

//std::map<DeviceCommandId_t, std::string> ArduinoHandler::awesomeMap =
//        ArduinoHandler::create_arduino_map();
