#include <bsp_sam9g20/devices/pvch/PvchWorker.h>
#include <fsfw/controller/ExtendedControllerBase.h>

/**
 * @brief       Device Handler for the DLR PVCH device
 * @details
 * Documentation of device: Internal datasheet, see KSat cloud or ask payload
 * subsystem member.
 *
 * @author      R. Mueller
 * @ingroup     devices
 */
class PVCHHandler: public ExtendedControllerBase {
public:
	PVCHHandler(object_id_t objectId, object_id_t pvchWorkerId, uint8_t switchId);
	virtual~ PVCHHandler();
protected:
	/* DeviceHandlerBase abstract function implementation */
//	void doStartUp() override;
//	void doShutDown() override;
//	ReturnValue_t buildNormalDeviceCommand(DeviceCommandId_t * id) override;
//	ReturnValue_t buildTransitionDeviceCommand(DeviceCommandId_t * id) override;
//	ReturnValue_t buildCommandFromCommand(DeviceCommandId_t deviceCommand,
//			const uint8_t * commandData, size_t commandDataLen) override;
//	void fillCommandAndReplyMap() override;
//	ReturnValue_t scanForReply(const uint8_t *start, size_t remainingSize,
//			DeviceCommandId_t *foundId, size_t *foundLen) override;
//	ReturnValue_t interpretDeviceReply(DeviceCommandId_t id,
//			const uint8_t *packet) override;
//	void setNormalDatapoolEntriesInvalid() override;

	/* DeviceHandlerBase overrides */
//	void debugInterface(uint8_t positionTracker = 0,
//			object_id_t objectId = 0, uint32_t parameter = 0) override;
//	uint32_t getTransitionDelayMs(Mode_t modeFrom, Mode_t modeTo) override;
//	ReturnValue_t getSwitches(const uint8_t **switches,
//			uint8_t *numberOfSwitches) override;
private:
	PvchWorker pvchWorker;
	const uint8_t switchId;
};

