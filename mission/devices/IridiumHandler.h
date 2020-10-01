#ifndef MISSION_DEVICES_IRIDIUMHANDLER_H_
#define MISSION_DEVICES_IRIDIUMHANDLER_H_

#include <fsfw/devicehandlers/DeviceHandlerBase.h>

/**
 * @brief   Device handler for the Iridium 9603 communication device.
 * @details
 * List of commands:
 * https://www.rock7.com/downloads/ATC_Iridium_ISU_AT_Command_Reference_MAN0009_v5.pdf
 */
class IridiumHandler: public DeviceHandlerBase {
public:
    IridiumHandler(object_id_t objectId, object_id_t deviceCommunication,
            CookieIF* comCookie);

private:
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

};



#endif /* MISSION_DEVICES_IRIDIUMHANDLER_H_ */
