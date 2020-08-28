/**
 * @file ArduinoDeviceHandler.h
 *
 * @date 06.03.2020
 * @author R. Mueller
 */

#ifndef TEST_TESTDEVICES_ARDUINODEVICEHANDLER_H_
#define TEST_TESTDEVICES_ARDUINODEVICEHANDLER_H_

#include <fsfw/devicehandlers/DeviceHandlerBase.h>

#include <vector>
#include <map>
#include <string>

/**
 * @brief Basic arduino device handler to test device commanding with an Arduino.
 * @details
 * @ingroup devices
 */
class ArduinoHandler: public DeviceHandlerBase {
public:
	ArduinoHandler(object_id_t objectId_, object_id_t comIF_,
			CookieIF * cookie_, std::string idString_);
	virtual ~ArduinoHandler();
protected:
	virtual void performOperationHook();
	/**
	 *  Perform startup procedure
	 */
	virtual void doStartUp();

	/**
	 * Shutdown procedure
	 */
	virtual void doShutDown();

	/**
	 *
	 * @param id
	 * @return
	 */
	virtual ReturnValue_t buildNormalDeviceCommand(DeviceCommandId_t * id);

	/**
	 *
	 * @param id
	 * @return
	 */
	virtual ReturnValue_t buildTransitionDeviceCommand(DeviceCommandId_t * id);

	/**
	 *
	 * @param deviceCommand
	 * @param commandData
	 * @param commandDataLen
	 * @return
	 */
	virtual ReturnValue_t buildCommandFromCommand(
			DeviceCommandId_t deviceCommand, const uint8_t * commandData,
			size_t commandDataLen);

	/**
	 * Fill in all required commands and expected replies by using the
	 * helper functions from
	 * @c DeviceHandlerBase
	 */
	virtual void fillCommandAndReplyMap();

	virtual void printCommand(uint8_t* command, size_t command_size);
	/**
	 * Check reply format.
	 * @param start
	 * @param len
	 * @param foundId Reply ID related to the request ID
	 * @param foundLen Found payload length
	 * @return
	 */
	virtual ReturnValue_t scanForReply(const uint8_t *start,
			size_t remainingSize, DeviceCommandId_t *foundId, size_t *foundLen);

	virtual void printReply(uint8_t * reply, size_t reply_size);

	/**
	 * Interpret reply content.
	 * @param id ID determined by scanForReply()
	 * @param packet
	 * @return
	 */
	virtual ReturnValue_t interpretDeviceReply(DeviceCommandId_t id,
			const uint8_t *packet);

	virtual void setNormalDatapoolEntriesInvalid();

	/**
	 * This needs to be set so DHB knows how long the start-up procedure
	 * will take
	 * @param modeFrom
	 * @param modeTo
	 * @return
	 */
	virtual uint32_t getTransitionDelayMs(Mode_t modeFrom, Mode_t modeTo);

	ReturnValue_t initialize() override;

	virtual ReturnValue_t getSwitches(const uint8_t **switches,
			uint8_t *numberOfSwitches);

	static const uint16_t MAX_REPLY_LEN {32};
	// Not needed for separate write/read
	//static const portTickType WRITE_READ_DELAY = 2;

	// Optional: Central map for all arduino device handlers.
	// However, thread-safety becomes an issue.
	//static std::map<DeviceCommandId_t, std::string> awesomeMap;
	std::map<DeviceCommandId_t, std::string> awesomeMap;
	std::map<DeviceCommandId_t, std::string>::iterator awesomeMapIter;

	static constexpr DeviceCommandId_t TEST_COMMAND_0 { 0 };
	static constexpr DeviceCommandId_t TEST_COMMAND_1 { 1 };
	static constexpr DeviceCommandId_t TEST_COMMAND_2 { 2 };
	static constexpr DeviceCommandId_t TEST_COMMAND_3 { 3 };

	std::string idString;
#if __cplusplus > 201703L
	static inline std::string string0 = "Hello, this is a message!";
	static inline std::string string1 = "Second message!";
	static inline std::string string2 = "Wow, amazing!";
	static inline std::string string3 = "SOURCE rocks!";
#else
	std::string string0 = "Hello, this is a message!";
	std::string string1 = "Second message!";
	std::string string2 = "Wow, amazing!";
	std::string string3 = "SOURCE rocks!";
#endif

	DeviceCommandId_t lastCommand{ 0 };
	std::vector<uint8_t> sendData { 0 };
	size_t receiveDataSize { 0 };

	uint16_t commandSendInterval { 1 };

	uint16_t commandSendCounter { 1 };
	uint16_t commandPrintCounter { 1 };
	uint16_t commandPrintInterval { 1 };

	// Some initial waiting time
	bool wait = false;

	// needed for static map initialization.
//	static std::map<DeviceCommandId_t, std::string> create_arduino_map() {
//	    std::map<DeviceCommandId_t, std::string> init_map;
//	    awesomeMap.emplace(TEST_COMMAND_0, string0);
//	    awesomeMap.emplace(TEST_COMMAND_1, string1);
//	    awesomeMap.emplace(TEST_COMMAND_2, string2);
//	    awesomeMap.emplace(TEST_COMMAND_3, string3);
//	    return init_map;
//	}
};

#endif /* TEST_TESTDEVICES_ARDUINODEVICEHANDLER_H_ */
