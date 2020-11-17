#ifndef MISSION_DEVICES_GYROHANDLER_H_
#define MISSION_DEVICES_GYROHANDLER_H_

#include "devicedefinitions/GyroPackets.h"
#include <fsfw/devicehandlers/DeviceHandlerBase.h>
#include <fsfw/globalfunctions/PeriodicOperationDivider.h>

/**
 * @brief       Device Handler for the BMG250 Gyroscope device
 * @details
 * Documentation of device:
 * https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bmg250-ds000.pdf
 * @author      R. Mueller
 * @ingroup     devices
 */
class GyroHandler: public DeviceHandlerBase {
public:
    GyroHandler(object_id_t objectId, object_id_t comIF, CookieIF * comCookie,
            uint8_t switchId);
    virtual~ GyroHandler();

    enum class CommInterface {
    	I2C,
		SPI
    };

    CommInterface comInterface = CommInterface::SPI;

    //! Gyro range (-+) in degrees
    static constexpr float GYRO_RANGE = 125.0;
    //! Check every week for now.
    static constexpr uint32_t SELF_TEST_PERIOD_SECOND = 60 * 60 * 24 * 7;
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
	ReturnValue_t initializeLocalDataPool(LocalDataPool& localDataPoolMap,
	        LocalDataPoolManager& poolManager) override;
	void modeChanged(void) override;


private:
    const uint8_t switchId;

    enum class InternalStates {
    	NONE,
		MODE_SELECT,
		WRITE_POWER,
		READ_PMU_STATUS,
		WRITE_RANGE,
		WRITE_CONFIG,
		READ_CONFIG,
		PERFORM_SELFTEST,
		READ_STATUS,
		RUNNING,
		FAULTY
    };
    InternalStates internalState = InternalStates::NONE;

    bool commandExecuted = false;
    bool checkSelfTestRegister = true;
    uint32_t lastSelfTestSeconds = 0;
    uint8_t selfTestFailCounter = 0;

    // The current gyro configuration will be cached here. It can be different
    // from the default configuration if the mode is changed by a command.
    uint8_t gyroConfiguration[2];
    uint8_t commandBuffer[12] = {};

    GyroDefinitions::GyroPrimaryDataset gyroData;
    GyroDefinitions::GyroAuxilliaryDataset gyroConfigSet;

    PeriodicOperationDivider selfTestDivider;
#if OBSW_ENHANCED_PRINTOUT == 1
	PeriodicOperationDivider* debugDivider;
#endif
};

#endif /* MISSION_DEVICES_GYROHANDLER_H_ */
