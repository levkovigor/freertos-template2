#ifndef MISSION_DEVICES_DEVICEPACKETS_GPSPACKETS_H_
#define MISSION_DEVICES_DEVICEPACKETS_GPSPACKETS_H_

#include <fsfw/serialize/EndianConverter.h>
#include <fsfw/serialize/SerializeElement.h>
#include <fsfw/serialize/SerialLinkedListAdapter.h>

/**
 * @brief 	This file contains all GPS direct commands and
 * 			the structure of received GPS data.
 */
class GpsNavigationMessage: public SerialLinkedListAdapter<SerializeIF> {
public:
	GpsNavigationMessage() {
		setLinks();
	}
	SerializeElement<uint16_t> startOfSequence; //!< GPSHandler::START_OF_SEQUENCE
	SerializeElement<uint16_t> payloadSize;
	SerializeElement<uint8_t> messageId; //!< DeviceCommandId_t GPSHandler::NAVIGATION_DATA_MESSAGE
	SerializeElement<uint8_t> fixMode; //!< enum GPSHandler::fixMode
	SerializeElement<uint8_t> numberOfSvInFix;
	SerializeElement<uint16_t> gnssWeek;
	SerializeElement<uint32_t> timeOfWeek;
	SerializeElement<int32_t> latitude;
	SerializeElement<int32_t> longitude;
	SerializeElement<uint32_t> ellipsoidAltitude;
	SerializeElement<uint32_t> meanSeaLevelAltitude;
	SerializeElement<uint16_t> gdop;
	SerializeElement<uint16_t> pdop;
	SerializeElement<uint16_t> hdop;
	SerializeElement<uint16_t> vdop;
	SerializeElement<uint16_t> tdop;
	SerializeElement<int32_t> ecefX;
	SerializeElement<int32_t> ecefY;
	SerializeElement<int32_t> ecefZ;
	SerializeElement<int32_t> ecefVx;
	SerializeElement<int32_t> ecefVy;
	SerializeElement<int32_t> ecefVz;
	SerializeElement<uint8_t> checksum;
	SerializeElement<uint16_t> endOfSequence; //!< GPSHandler::END_OF_SEQUENCE
private:
	//Forbid copying because of next pointer to member
	GpsNavigationMessage(const GpsNavigationMessage &message);
	void setLinks() {
		setStart(&startOfSequence);
		startOfSequence.setNext(&payloadSize);
		payloadSize.setNext(&messageId);
		messageId.setNext(&fixMode);
		fixMode.setNext(&numberOfSvInFix);
		numberOfSvInFix.setNext(&gnssWeek);
		gnssWeek.setNext(&timeOfWeek);
		timeOfWeek.setNext(&latitude);
		latitude.setNext(&longitude);
		longitude.setNext(&ellipsoidAltitude);
		ellipsoidAltitude.setNext(&meanSeaLevelAltitude);
		meanSeaLevelAltitude.setNext(&gdop);
		gdop.setNext(&pdop);
		pdop.setNext(&hdop);
		hdop.setNext(&vdop);
		vdop.setNext(&tdop);
		tdop.setNext(&ecefX);
		ecefX.setNext(&ecefY);
		ecefY.setNext(&ecefZ);
		ecefZ.setNext(&ecefVx);
		ecefVx.setNext(&ecefVy);
		ecefVy.setNext(&ecefVz);
		ecefVz.setNext(&checksum);
		checksum.setNext(&endOfSequence);
	}
};

struct GpsSetBaudRate{ //!< [EXPORT] : [COMMAND] GPSHandler : SET_GPS_BAUDRATE
	uint8_t baudRateSelect; //!< [EXPORT] : [ENUM] baudRateSelect
	uint8_t attributes; //!< [EXPORT] : [ENUM] attributes
};

struct GpsSetUpdateRate{ //!< [EXPORT] : [COMMAND] GPSHandler : SET_GPS_UPDATE_RATE
	uint8_t updateRate; //!< [EXPORT] : [ENUM] updateRates
	uint8_t attributes; //!< [EXPORT] : [ENUM] attributes
};

struct GpsSetMessageType{ //!< [EXPORT] : [COMMAND] GPSHandler : SET_GPS_MESSAGE_TYPE
	//!< [EXPORT] : [COMMENT] Change GPS periodic reply message
	uint8_t type; //!< [EXPORT] : [ENUM] messageType
	uint8_t attributes; //!<  [EXPORT] : [ENUM] attributes
};


class GpsRestartCommand: public SerialLinkedListAdapter<SerializeIF> { //!< [EXPORT] : [COMMAND] GPSHandler : RESTART_DEVICE
//!< [EXPORT] : [COMMENT] Device will restart with the specified initial values
public:
	GpsRestartCommand() {
		setLinks();
	}
private:
	//Forbid copying because of next pointer to member
	GpsRestartCommand(const GpsRestartCommand &command);
	void setLinks() {
		setStart(&startMode);
		startMode.setNext(&utcYear);
		utcYear.setNext(&utcMonth);
		utcMonth.setNext(&utcDay);
		utcDay.setNext(&utcMinute);
		utcMinute.setNext(&latitude);
		latitude.setNext(&longitude);
		longitude.setNext(&altitude);
	}

	SerializeElement<uint8_t> startMode;  //!< [EXPORT] : [ENUM] restartMode
	SerializeElement<uint16_t> utcYear;
	SerializeElement<uint8_t> utcMonth;
	SerializeElement<uint8_t> utcDay;
	SerializeElement<uint8_t> utcMinute;
	SerializeElement<uint16_t> latitude; //!< [EXPORT] : [COMMENT] <add some format information here>
	SerializeElement<uint16_t> longitude; //!< [EXPORT] : [COMMENT] <add some format information here>
	SerializeElement<uint16_t> altitude; //!< [EXPORT] : [COMMENT] <add some format information here>

};

struct GpsResetCommand { //!< [EXPORT] : [COMMAND] GPSHandler : RESET_DEVICE
	uint8_t resetType; //!< [EXPORT] : [ENUM] resetType
};


#endif /* MISSION_DEVICES_DEVICEPACKETS_GPSPACKETS_H_ */
