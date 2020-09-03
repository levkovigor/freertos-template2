#ifndef MISSION_DEVICES_GYROHANDLER_H_
#define MISSION_DEVICES_GYROHANDLER_H_
#include <fsfw/devicehandlers/DeviceHandlerBase.h>

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
    // Check every week for now.
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

    ReturnValue_t initialize() override;

private:
    const uint8_t switchId;

    enum class InternalState {
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
    InternalState internalState = InternalState::NONE;
    bool commandExecuted = false;
    bool checkSelfTestRegister = true;
    uint8_t selfTestCycleCounterTrigger = 5;
    uint8_t selfTestCycleCounter = 0;
    uint32_t lastSelfTestSeconds = 0;
    uint8_t selfTestFailCounter = 0;

    // The current gyro configuration will be cached here. It can be different
    // from the default configuration if the mode is changed by a command.
    uint8_t gyroConfiguration[2];
    uint8_t commandBuffer[12] = {};

    static constexpr uint8_t SPI_MODE_SELECT = 0x7F;
    static constexpr uint8_t POWER_REGISTER = 0x7E;
    static constexpr uint8_t PMU_REGISTER = 0x03;
    static constexpr uint8_t STATUS_REGISTER = 0x1B;
    static constexpr uint8_t CONFIG_REGISTER = 0x42;
    static constexpr uint8_t RANGE_REGISTER = 0x43;
    static constexpr uint8_t SELFTEST_REGISTER = 0x6D;
    // Normal PMU mode for Gyroscope.
    static constexpr uint8_t POWER_CONFIG = 0x15;


    // Configuration for minimal range (+-125 degree).
    static constexpr uint8_t RANGE_CONFIG = 0b0000'0100;
    // To start self-test, write this byte to the selftest register
    static constexpr uint8_t PERFORM_SELFTEST_CMD = 0b0001'0000;
    // Configure OSR2, oversampling rate 2 (see p.15 of datasheet)
    static constexpr uint8_t GYRO_BWP_CONFIG = 0b0001'0000;
    // Configure 50 Hz output data rate, equals 10.7 Hz 3dB cutoff frequency
    // with OSR2.
    static constexpr uint8_t GYRO_ODR_CONFIG = 0b0000'0111;
    // Default configuration.
    static constexpr uint8_t GYRO_DEF_CONFIG = GYRO_BWP_CONFIG | GYRO_ODR_CONFIG;

    static constexpr uint8_t SELFTEST_OK = 0b0000'0010;


    // Data register X-Axis
    static constexpr uint8_t DATA_REGISTER_START = 0x12;

    // Gyroscope read mask
    static  constexpr uint8_t GYRO_READ_MASK = 0x80;

    static constexpr DeviceCommandId_t SPI_SELECT = SPI_MODE_SELECT;
    static constexpr DeviceCommandId_t WRITE_POWER = POWER_REGISTER;
    static constexpr DeviceCommandId_t WRITE_RANGE = RANGE_REGISTER;
    static constexpr DeviceCommandId_t WRITE_CONFIG = CONFIG_REGISTER;
    static constexpr DeviceCommandId_t READ_CONFIG = CONFIG_REGISTER | GYRO_READ_MASK;
    static constexpr DeviceCommandId_t READ_PMU = PMU_REGISTER | GYRO_READ_MASK;
    static constexpr DeviceCommandId_t READ_STATUS = STATUS_REGISTER | GYRO_READ_MASK;
    static constexpr DeviceCommandId_t PERFORM_SELFTEST = SELFTEST_REGISTER;
    static constexpr DeviceCommandId_t GYRO_DATA = DATA_REGISTER_START | GYRO_READ_MASK;

    uint8_t counter = 20;
};

#endif /* MISSION_DEVICES_GYROHANDLER_H_ */
