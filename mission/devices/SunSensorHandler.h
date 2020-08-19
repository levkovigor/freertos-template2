#ifndef MISSION_DEVICES_SUNSENSORHANDLER_H_
#define MISSION_DEVICES_SUNSENSORHANDLER_H_
#include <fsfw/devicehandlers/DeviceHandlerBase.h>

/**
 * @brief       Device Handler for the OSRAM SFH2430 sun sensor device
 * @details
 * Documentation of device: Datasheet available in the internet.
 *
 * The sun sensors raw analog values are amplified with the OPA4196
 * operational amplifier and then converted to digital values using
 * the MAX1229 ADC device. In that sense, the ADC device is interface to the
 * sun sensors. For more information (and the truth, should this
 * comment ever become outdated), refer to the SOURCE system schematic.
 *
 * @author      R. Mueller
 * @ingroup     devices
 */
class SunSensorHandler: public DeviceHandlerBase {
public:
    SunSensorHandler(object_id_t objectId, object_id_t comIF, CookieIF * comCookie,
            uint8_t switchId);
    virtual~ SunSensorHandler();

protected:
    /* DeviceHandlerBase abstract function implementation */
    void doStartUp() override;
    void doShutDown() override;
    ReturnValue_t buildNormalDeviceCommand(DeviceCommandId_t * id) override;
    ReturnValue_t buildTransitionDeviceCommand(DeviceCommandId_t * id) override;
    ReturnValue_t buildCommandFromCommand(DeviceCommandId_t deviceCommand,
            const uint8_t * commandData, size_t commandDataLen) override;
    void fillCommandAndReplyMap() override;
    ReturnValue_t scanForReply(const uint8_t *start, size_t remainingSize,
            DeviceCommandId_t *foundId, size_t *foundLen) override;
    ReturnValue_t interpretDeviceReply(DeviceCommandId_t id,
            const uint8_t *packet) override;
    void setNormalDatapoolEntriesInvalid() override;

    /* DeviceHandlerBase overrides */
    void debugInterface(uint8_t positionTracker = 0,
            object_id_t objectId = 0, uint32_t parameter = 0) override;
    uint32_t getTransitionDelayMs(Mode_t modeFrom, Mode_t modeTo) override;
    ReturnValue_t getSwitches(const uint8_t **switches,
            uint8_t *numberOfSwitches) override;
private:
    const uint8_t switchId;
};

#endif /* MISSION_DEVICES_SUNSENSORHANDLER_H_ */
