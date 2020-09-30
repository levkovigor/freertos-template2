#ifndef MISSION_DEVICES_GPSHANDLER_H_
#define MISSION_DEVICES_GPSHANDLER_H_

#include "devicedefinitions/GPSPackets.h"

#include <fsfw/datapoolglob/GlobalDataSet.h>
#include <fsfw/datapoolglob/GlobalPoolVariable.h>
#include <fsfw/datapoolglob/GlobalPoolVector.h>
#include <fsfw/devicehandlers/DeviceHandlerBase.h>

/**
 * @brief		Device handler for the Venus838 GPS device.
 * @details
 * Documentation of binary messages:
 * Application Note AN0028 for SkyTraq Venus 8 GNSS Receiver
 * @author		R. Mueller
 * @ingroup 	devices
 */
class GPSHandler : public DeviceHandlerBase {
public:
	GPSHandler(object_id_t objectId, object_id_t comIF, CookieIF * cookie,
			uint8_t powerSwitchId);
	virtual ~GPSHandler();

	static uint8_t calcChecksum(const uint8_t * payload,uint16_t payloadSize);

	static constexpr uint8_t BINARY_HEADER_SIZE = 4;
	static constexpr uint8_t BINARY_HEADER_AND_TAIL_SIZE = 7;

protected:
	/* DHB abstract function implementation */
	/** Perform startup procedure */
	void doStartUp() override;
	/** Shutdown procedure is not needed for GPS Device */
	void doShutDown() override;

	/** Normal mode is not used for GPS device.*/
	ReturnValue_t buildNormalDeviceCommand(DeviceCommandId_t * id) override;
	/**
	 * Build commands for transition procedures like startup for the GPS device
	 * @param id [out] ID of command to send.
	 */
	ReturnValue_t buildTransitionDeviceCommand(DeviceCommandId_t * id) override;
	/**
	 * Build command from Service 8 Function Management Command, using
	 * the hasActionIF and ActionHelper
	 * @param deviceCommand
	 * @param commandData
	 * @param commandDataLen
	 * @return
	 */
	ReturnValue_t buildCommandFromCommand(DeviceCommandId_t deviceCommand,
			const uint8_t * commandData,size_t commandDataLen) override;

	/**
	 * Fill in all required commands and expected replies by using the helper
	 * functions from @c DeviceHandlerBase
	 */
	void fillCommandAndReplyMap() override;

	/**
	 * Check whether a received GPS packet has the correct format
	 * (checksum and packet length)
	 * Scans for the Reply ID and the payload length and assigns values
	 * @param start
	 * @param len
	 * @param foundId Reply ID related to the request ID
	 * @param foundLen Found payload length
	 * @return
	 */
	ReturnValue_t scanForReply(const uint8_t *start, size_t len,
			DeviceCommandId_t *foundId, size_t *foundLen) override;
	/**
	 * Interpret reply content. Determines ACK or NACK message type for binary
	 * replies and extracts data from NMEA strings or binary message to be
	 * stored in the datapool.
	 * @param id ID determined by scanForReply()
	 * @param packet
	 * @return
	 */
	ReturnValue_t interpretDeviceReply(DeviceCommandId_t id,
			const uint8_t *packet) override;
	/** This is not needed, GPS does not run in normal mode */
	    void setNormalDatapoolEntriesInvalid() override;

	/* DHB virtual function overrides */
	/**
	 * This needs to be set so DHB knows how long the start-up procedure
	 * will take.
	 * @param modeFrom
	 * @param modeTo
	 * @return
	 */
	uint32_t getTransitionDelayMs(Mode_t modeFrom, Mode_t modeTo) override;
	ReturnValue_t getSwitches(const uint8_t **switches,
			uint8_t *numberOfSwitches) override;

private:
	const uint8_t switchId;

	static constexpr uint32_t STARTUP_TIMEOUT_MS = 15000;
	static constexpr uint32_t SHUTDOWN_TIMEOUT_MS = 15000;

	static const uint32_t MAX_REPLY_LENGTH = 256;
	static const uint32_t MAX_GPS_PACKET_LENGTH = 66;
	//! A GPS packet with a payload length of 0 has this minimum length
	static const uint8_t MIN_REPLY_LENGTH = 7;

	static const uint16_t START_OF_SEQUENCE = 0xA0A1;
	static const uint16_t END_OF_SEQUENCE = 0x0D0A;
	static const uint16_t NMEA_START_OF_SEQUENCE = 0x2447; //!< string "$G"
	static const uint8_t NMEA_TYPE_SIZE = 3;

	static const uint8_t PAYLOAD_START_INDEX = 4;

	static constexpr uint16_t MAX_COMMAND_LENGTH = 100;
	uint8_t commandBuffer[MAX_COMMAND_LENGTH];

	/* Specify default intervals for NMEA strings. 0->Disabled, Range 0 - 255 */
	static const uint8_t ggaSelect = 1;
	static const uint8_t gsaSelect = 0;
	static const uint8_t gsvSelect = 0;
	static const uint8_t gllSelect = 0;
	static const uint8_t rmcSelect = 1;
	static const uint8_t vtgSelect = 0;
	static const uint8_t zdaSelect = 0;

	/**
	 *  Start-Up Flags used to configure device
	 */
	enum class InternalState {
		STATE_NONE,
		STATE_WAIT_FIRST_MESSAGE,
		STATE_SET_UPDATE_RATE,
		STATE_SET_MESSAGE_TYPE,
		STATE_CONFIGURED
	};
	InternalState internalState = InternalState::STATE_NONE;

	/* Command Packet IDs which will be parsed by exporter */
	static const DeviceCommandId_t RESTART_DEVICE = 0x1; //!< [EXPORT] : [COMMAND]
	static const DeviceCommandId_t RESET_DEVICE = 0x4; //!< [EXPORT] : [COMMAND]
	static const DeviceCommandId_t SET_GPS_BAUDRATE = 0x5; //!< [EXPORT] : [COMMAND]
	static const DeviceCommandId_t SET_GPS_NMEA_OUTPUT = 0x8; //!< [EXPORT] : [COMMAND]
	static const DeviceCommandId_t SET_GPS_MESSAGE_TYPE = 0x9; //!< [EXPORT] : [COMMAND]
	static const DeviceCommandId_t SET_GPS_UPDATE_RATE = 0xE; //!< [EXPORT] : [COMMAND]

	/* Reply IDs */
	static const DeviceCommandId_t NAVIGATION_DATA_MESSAGE = 0xA8; //!< Binary GPS data packet ID
	static const DeviceCommandId_t ACK = 0x83;
	static const DeviceCommandId_t NACK = 0x84;
	static const uint8_t NMEA_MESSAGE = 200; //!< Custom value, NMEA does not have a proper ID

	enum baudRateSelect { //!< [EXPORT] : [ENUM]
		RATE_4800 = 0,
		RATE_9600 = 1, //!< Default value
		RATE_19200 = 2,
		RATE_38400 = 3,
		RATE_57600 = 4,
		RATE_115200 = 5,
		RATE_230400 = 6,
		RATE_460800 = 7,
		RATE_921600 = 8
	};


	enum attributes { //!< [EXPORT] : [ENUM]
		SRAM = 0, //!< Default value
		SRAM_AND_FLASH = 1,
		TEMPORARILY = 2
	};

	enum messageType { //!< [EXPORT] : [ENUM]
		NO_MESSAGE = 0,
		NMEA = 1,
		BINARY = 2 //!< Preferred output for now, contains all important information and no ASCII parsing needed

	};

	enum restartMode { //!< [EXPORT] : [ENUM]
		HOT_START = 1, //!< Takes about one second
		WARM_START = 2,
		COLD_START = 3 //!< Takes 29 seconds
	};

	enum resetType { //!< [EXPORT] : [ENUM]
		REGULAR_RESET = 0,
		RESTART_AFTER_RESET = 1
	};

	enum updateRates { //!< [EXPORT] : [ENUM]
		RATE1 = 1, //!< Default value
		RATE2 = 2,
		RATE4 = 4,
		RATE5 = 5,
		RATE8 = 8,
		RATE10 = 10,
		RATE20 = 20
	};

	/* Configuration Parameters for Startup procedure */
	static const uint8_t defaultMessageType = messageType::BINARY;
	static const uint8_t defaultBaudSelect = baudRateSelect::RATE_9600;
	static const uint8_t defaultGNSSupdateRate = 2; //!< update rate in Hz. Possible Values: 1,2,4,5,8,10,20,25,40,50

	static const uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::GPS_DEVICE;
	static const Event GPS_STARTUP_FAILED = MAKE_EVENT(0, SEVERITY::LOW); //!< Startup failed. P1=0 -> doStartup() function bool problem P2=0
	static const Event GPS_FIX = MAKE_EVENT(1,SEVERITY::INFO);
	static const Event GPS_LOST_FIX = MAKE_EVENT(2,SEVERITY::INFO);


	bool commandExecuted; //!< Used to confirm start-up command acknowledgement
	bool firstReplyReceived;

	GpsNavigationMessage navMessage;

	ReturnValue_t buildBaudSelectCommand(const uint8_t *commandData,
			size_t commandDataLen);
	ReturnValue_t buildMessageTypeSelectCommand(const uint8_t * commandData,
			size_t commandDataLen);
	ReturnValue_t buildGpsUpdateRateCommand(const uint8_t * commandData,
			size_t commandDataLen);
	void prepareGpsCommandHeaderAndTail(DeviceCommandId_t deviceCommand,
			uint8_t * sizeOfCommandToBuild, uint16_t * payloadLength);

	ReturnValue_t checkBinaryReply(const uint8_t *start, size_t * len,
			DeviceCommandId_t *foundId, size_t *foundLen);
	ReturnValue_t interpretNavigationData(const uint8_t *packet);

	void deSerializeNavigationDataIntoDataset(const uint8_t * packet);
	void checkAndStoreStructDataIntoDatapool();
};

#endif /* MISSION_DEVICES_GPSHANDLER_H_ */
