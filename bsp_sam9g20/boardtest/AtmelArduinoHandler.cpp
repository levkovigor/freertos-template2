#include <bsp_sam9g20/boardtest/AtmelArduinoHandler.h>
#include <bsp_sam9g20/comIF/cookies/SpiCookie.h>

AtmelArduinoHandler::AtmelArduinoHandler(object_id_t objectId,
		object_id_t comIF, CookieIF *cookie, uint8_t deviceSwitch,
		std::string idString):
		ArduinoHandler(objectId, comIF, cookie, idString) {
	CookieIF * my_cookie = dynamic_cast<I2cCookie*>(cookie);
	if(my_cookie != nullptr) {
		comType = I2C;
		// Alternatively, consecutive write read can be set here.
		setI2cComType(I2cCommunicationType::SEPARATE_WRITE_READ);
	}
}

AtmelArduinoHandler::~AtmelArduinoHandler() {}

void AtmelArduinoHandler::doStartUp() {
    ArduinoHandler::doStartUp();
    // Set Mode to normal so normal device commands are built.
    setMode(MODE_NORMAL);
    // wait once to prevent weird print output at start-up.
    if(wait) {
        wait = false;
        vTaskDelay(50);
    }
}

ReturnValue_t AtmelArduinoHandler::buildNormalDeviceCommand(
        DeviceCommandId_t* id) {
    ReturnValue_t result = ArduinoHandler::buildNormalDeviceCommand(id);
    if(commandSendCounter == commandSendInterval) {
        if(comType == I2C and
           getI2cComType() == I2cCommunicationType::CONSECUTIVE_WRITE_READ)
        {
            // little bit hacked.
            setI2cReceiveDataSize(rawPacketLen);
        }
    }
    return result;
}

void AtmelArduinoHandler::setI2cComType(I2cCommunicationType i2cComType) {
    if(comType != I2C) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "Arduino Handler: Invalid ComType!" << std::endl;
#else
#endif
        return;
    }
    I2cCookie * i2cCookie = dynamic_cast<I2cCookie *>(comCookie);
    if(i2cCookie == nullptr) {
        return;
    }
    i2cCookie->setI2cComType(i2cComType);
}

I2cCommunicationType AtmelArduinoHandler::getI2cComType() const {
    if(comType != I2C) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "Arduino Handler: Invalid ComType!" << std::endl;
#else
#endif
        return I2cCommunicationType::UNKNOWN;
    }
    I2cCookie* i2cCookie = dynamic_cast<I2cCookie*>(comCookie);
    if(i2cCookie != nullptr) {
        return i2cCookie->getI2cComType();
    }
    return I2cCommunicationType::UNKNOWN;
}

void AtmelArduinoHandler::setI2cReceiveDataSize(size_t receiveDataSize) {
	if(comType != I2C) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "Arduino Handler: Invalid ComType!" << std::endl;
#else
#endif
		return;
	}
	I2cCookie* i2cCookie = dynamic_cast<I2cCookie *>(comCookie);
	if(i2cCookie == nullptr) {
	    return;
	}
	i2cCookie->setReceiveDataSize(receiveDataSize);
}

ReturnValue_t AtmelArduinoHandler::initialize() {
	ReturnValue_t result = ArduinoHandler::initialize();
	if(result == RETURN_OK) {
		CookieIF * spiCookie = dynamic_cast<SpiCookie*>(comCookie);
		if(spiCookie != nullptr) {
			comType = SPI;
			setSpiMessages();
			// If using static map.
//			taskENTER_CRITICAL();
//			if(not spiMessageAdapted) {
//			    setSpiMessages();
//			    spiMessageAdapted = true;
//			    taskEXIT_CRITICAL();
		}
	}
	return result;
}

void AtmelArduinoHandler::printReply(uint8_t *reply, size_t reply_size) {
    if(comType == ComInterfaceType::SPI) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info  << "Arduino Handler received echo reply from " << idString << ": "
                << reply + 1  << std::endl;
#else
#endif
    }
    else {
        ArduinoHandler::printReply(reply, reply_size);
    }
}

void AtmelArduinoHandler::setSpiMessages() {
    for(;awesomeMapIter != ArduinoHandler::awesomeMap.end();awesomeMapIter ++) {
        std::string & currentString = awesomeMapIter->second;
        //currentString.insert(0, 1, 's');
        currentString.append(std::string("\r\n"));
    }
    //sif::info << "Arduino Handler: Adapted strings for SPI" << std::endl;
}
