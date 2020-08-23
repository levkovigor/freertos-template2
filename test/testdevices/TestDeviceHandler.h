#ifndef TEST_TESTDEVICES_TESTDEVICEHANDLER_H_
#define TEST_TESTDEVICES_TESTDEVICEHANDLER_H_

#include <fsfw/devicehandlers/DeviceHandlerBase.h>
#include <fsfw/events/EventReportingProxyIF.h>
#include <test/testinterfaces/DummyCookie.h>

/**
 * @brief 	Basic dummy device handler to test device commanding without a
 * 			physical device.
 * @details
 * Use ActionMessages for functional commanding (PUS Service 8) or
 * Device Handler Messages for raw commanding (sent buffer is simply returned).
 * @author	R. Mueller
 * @ingroup devices
 */
class TestDevice: public DeviceHandlerBase {
public:
	TestDevice(object_id_t objectId, object_id_t comIF, CookieIF * cookie,
	        bool onImmediately = false);
	virtual ~ TestDevice();

	enum class PoolIds {
		TEST_VAR_1,
		TEST_VEC_1,
		TEST_VEC_2
	};

	virtual ReturnValue_t getParameter(uint8_t domainId, uint16_t parameterId,
			ParameterWrapper *parameterWrapper,
			const ParameterWrapper *newValues, uint16_t startAtIndex);

protected:
	void performOperationHook() override;
	virtual void doStartUp() override;
	virtual void doShutDown() override;
	/** No periodic commands for now */
	virtual ReturnValue_t buildNormalDeviceCommand(
			DeviceCommandId_t * id) override;
	/** No transition commands for now */
	virtual ReturnValue_t buildTransitionDeviceCommand(
			DeviceCommandId_t * id) override;

	/**
	 * Three commands have been implemented and can be tested by using service 8
	 * and sending object ID + action ID + data if required or needed.
	 * Refer to command parameters and command IDs !
	 * Commands are sent to a virtual com IF which simply returns sent data.
	 * Refer to Dummy Communication Interface for more information.
	 * @param deviceCommand
	 * @param commandData
	 * @param commandDataLen
	 * @return
	 */
	virtual ReturnValue_t buildCommandFromCommand(DeviceCommandId_t
			deviceCommand, const uint8_t * commandData,
			size_t commandDataLen) override;

	/**
	 * Possible commands and replies are inserted in this function.
	 */
	virtual void fillCommandAndReplyMap() override;

	/**
	 * The returned data (echo reply) can be checked for formatting, length
	 * or other parameters
	 * @param start
	 * @param len
	 * @param foundId
	 * @param foundLen
	 * @return
	 */
	virtual ReturnValue_t scanForReply(const uint8_t *start, size_t len,
			DeviceCommandId_t *foundId, size_t *foundLen) override;

	/**
	 * The returned data is interpreted. Action Messages are set to generate
	 * TC verification or data replies
	 * @param id
	 * @param packet
	 * @return
	 */
	virtual ReturnValue_t interpretDeviceReply(DeviceCommandId_t id,
			const uint8_t *packet) override;

	virtual void setNormalDatapoolEntriesInvalid() override;

	/**
	 * Used to track variables in performOperation() call of DHB
	 */
	virtual void debugInterface(uint8_t positionTracker = 0,
			object_id_t objectId = 0, uint32_t parameter = 0) override;

	/**
	 * Transition delay, used for transition from MODE_START_UP to MODE_ON
	 * @param modeFrom
	 * @param modeTo
	 * @return
	 */
	virtual uint32_t getTransitionDelayMs(Mode_t modeFrom,
			Mode_t modeTo) override;

	virtual void doTransition(Mode_t modeFrom, Submode_t subModeFrom) override;

private:
	static constexpr uint8_t MAX_BUFFER_SIZE = 255;

	static constexpr DeviceCommandId_t TEST_COMMAND_1 = 666; //!< Test completion reply
	static constexpr DeviceCommandId_t TEST_COMMAND_2 = 0xC0C0BABE; //!< Test data reply
	static constexpr DeviceCommandId_t TEST_COMMAND_3 = 0xBADEAFFE; //!< Test step and completion reply

	/**
	 * These parameters are sent back with the command ID as a data reply
	 */

	static constexpr uint16_t COMMAND_2_PARAM1 = 0xBAB0; //!< param1, 2 bytes
	//! param2, 8 bytes
	static constexpr uint64_t COMMAND_2_PARAM2 = 0x000000524F42494E;

	static constexpr size_t TEST_COMMAND_2_SIZE = sizeof(TEST_COMMAND_2) +
			sizeof(COMMAND_2_PARAM1) + sizeof(COMMAND_2_PARAM2);

	uint8_t commandBuffer[MAX_BUFFER_SIZE];
	uint16_t trigger = 0;
	bool onImmediately = false;
	bool oneShot = true;
	//variables for service 20 test
	uint32_t testParameter1 = 0;
	int32_t testParameter2 = -2;
	float testParameter3 = 3.3;

	ReturnValue_t buildTestCommand1(DeviceCommandId_t deviceCommand,
			const uint8_t* commandData, size_t commandDataLen);
	ReturnValue_t buildTestCommand2(DeviceCommandId_t deviceCommand,
			const uint8_t* commandData, size_t commandDataLen);
	ReturnValue_t buildTestCommand3(DeviceCommandId_t deviceCommand,
			const uint8_t* commandData, size_t commandDataLen);
	ReturnValue_t buildTestCommand4(DeviceCommandId_t deviceCommand,
			const uint8_t* commandData, size_t commandDataLen);

	ReturnValue_t interpretingReply1();
	ReturnValue_t interpretingReply2(DeviceCommandId_t id, const uint8_t* packet);
	ReturnValue_t interpretingReply3(DeviceCommandId_t id, const uint8_t* packet);

	ReturnValue_t initializePoolEntries(
			LocalDataPool& localDataPoolMap) override;

	//void testLocalDataPool();

	int counter = 0;
};



#endif /* TEST_TESTDEVICES_TESTDEVICEHANDLER_H_ */
