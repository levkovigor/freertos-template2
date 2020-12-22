#ifndef MISSION_DEVICES_PCDUHANDLER_H_
#define MISSION_DEVICES_PCDUHANDLER_H_

#include <fsfw/devicehandlers/DeviceHandlerBase.h>
#include <mission/fdir/PCDUFailureIsolation.h>
#include <fsfw/ipc/MessageQueueIF.h>


/**
 * @ingroup devices
 */
class PCDUHandler : public DeviceHandlerBase,
		public PowerSwitchIF,
		public ConfirmsFailuresIF {
public:
	PCDUHandler(object_id_t objectId, object_id_t comIF, CookieIF * cookie);
	virtual ~PCDUHandler();

protected:

	/**
	 * @brief DHB virtual abstract functions
	 */
	void doStartUp() override;
	void doShutDown() override;
	ReturnValue_t buildNormalDeviceCommand(DeviceCommandId_t * id) override;
	ReturnValue_t buildTransitionDeviceCommand(DeviceCommandId_t * id) override;
	ReturnValue_t buildCommandFromCommand(DeviceCommandId_t deviceCommand,
			const uint8_t * commandData,size_t commandDataLen) override;
	void fillCommandAndReplyMap() override;
	ReturnValue_t scanForReply(const uint8_t *start, size_t len,
			DeviceCommandId_t *foundId, size_t *foundLen) override;
	ReturnValue_t interpretDeviceReply(DeviceCommandId_t id,
			const uint8_t *packet) override;
	void setNormalDatapoolEntriesInvalid() override;
	uint32_t getTransitionDelayMs(Mode_t modeFrom, Mode_t modeTo) override;


	/**
	 * PowerSwitcherIF virtual functions
	 */
	void sendSwitchCommand(uint8_t switchNr, ReturnValue_t onOff) const override;
	void sendFuseOnCommand(uint8_t fuseNr) const override;
	ReturnValue_t getSwitchState(uint8_t switchNr) const override;
	ReturnValue_t getFuseState(uint8_t fuseNr) const override;
	uint32_t getSwitchDelayMs(void) const override;

	/**
	 * This is the power switcher confirms failure implementation
	 * which is important for the initialization of all other device handlers
	 * @return
	 */
	MessageQueueId_t getEventReceptionQueue() override;

private:
	static const uint8_t MAX_BUFFER_SIZE = 10;
	uint8_t commandBuffer[MAX_BUFFER_SIZE];


	static const DeviceCommandId_t TEST_COMMAND_1 = 666;
	static const DeviceCommandId_t TEST_COMMAND_2 = 0xCAFEAFFE;
	static const uint32_t MAX_REPLY_LENGTH = 256;
	MessageQueueIF* eventQueue;

	PCDUFailureIsolation fdir;

	ReturnValue_t switchList[40];

	enum test_enum { //!< [EXPORT] : [ENUM]
		TEST = 0, //!< Default value
	};
};



#endif /* MISSION_DEVICES_PCDUHANDLER_H_ */
