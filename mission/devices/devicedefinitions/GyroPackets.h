#ifndef MISSION_DEVICES_DEVICEPACKETS_GYROPACKETS_H_
#define MISSION_DEVICES_DEVICEPACKETS_GYROPACKETS_H_

#include <fsfw/datapoollocal/StaticLocalDataSet.h>
#include <fsfw/datapoollocal/LocalPoolVariable.h>
#include <fsfw/devicehandlers/DeviceHandlerIF.h>

namespace gyrodefs {

enum GyroPoolIds: lp_id_t {
    ANGULAR_VELOCITY_X,
    ANGULAR_VELOCITY_Y,
    ANGULAR_VELOCITY_Z,
    GENERAL_CONFIG_REG42,
    RANGE_CONFIG_REG43
};

static constexpr DeviceCommandId_t WRITE_CONFIG = 0x00;
static constexpr DeviceCommandId_t READ_CONFIG = 0x01;
static constexpr DeviceCommandId_t PERFORM_SELFTEST = 0x02;
static constexpr DeviceCommandId_t READ_STATUS = 0x03;
static constexpr DeviceCommandId_t GYRO_DATA = 0x04;

// Gyroscope read mask
static  constexpr uint8_t GYRO_READ_MASK = 0x80;

// Data register X-Axis
static constexpr uint8_t DATA_REGISTER_START = 0x12;

static constexpr uint8_t SPI_MODE_SELECT = 0x7F;
static constexpr uint8_t POWER_REGISTER = 0x7E;
static constexpr uint8_t PMU_REGISTER = 0x03;
static constexpr uint8_t STATUS_REGISTER = 0x1B;
static constexpr uint8_t CONFIG_REGISTER = 0x42;
static constexpr uint8_t GYRO_DATA_CMD = DATA_REGISTER_START | GYRO_READ_MASK;
static constexpr uint8_t READ_CONFIG_CMD = CONFIG_REGISTER | GYRO_READ_MASK;
static constexpr uint8_t READ_STATUS_CMD = STATUS_REGISTER | GYRO_READ_MASK;
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

static constexpr DeviceCommandId_t SPI_SELECT = SPI_MODE_SELECT;
static constexpr DeviceCommandId_t WRITE_POWER = POWER_REGISTER;
static constexpr DeviceCommandId_t WRITE_RANGE = RANGE_REGISTER;
static constexpr DeviceCommandId_t READ_PMU = PMU_REGISTER | GYRO_READ_MASK;

static constexpr uint32_t GYRO_DATA_SET_ID = gyrodefs::GYRO_DATA;
static constexpr uint32_t GYRO_AUX_SET_ID = gyrodefs::READ_CONFIG;

}

class GyroPrimaryDataset: public StaticLocalDataSet<3 * sizeof(float)> {
public:
    /**
     * Constructor for data users
     * @param gyroId
     */
    GyroPrimaryDataset(object_id_t gyroId):
            StaticLocalDataSet(sid_t(gyroId, gyrodefs::GYRO_DATA_SET_ID)) {
        setAllVariablesReadOnly();
    }

    lp_var_t<float> angVelocityX = lp_var_t<float>(sid.objectId,
            gyrodefs::ANGULAR_VELOCITY_X, this);
    lp_var_t<float> angVelocityY = lp_var_t<float>(sid.objectId,
            gyrodefs::ANGULAR_VELOCITY_Y, this);
    lp_var_t<float> angVelocityZ = lp_var_t<float>(sid.objectId,
            gyrodefs::ANGULAR_VELOCITY_Z, this);
private:

    friend class GyroHandler;
    /**
     * Constructor for data creator
     * @param hkOwner
     */
    GyroPrimaryDataset(HasLocalDataPoolIF* hkOwner):
            StaticLocalDataSet(hkOwner, gyrodefs::GYRO_DATA_SET_ID) {}
};

class GyroAuxilliaryDataset: public StaticLocalDataSet<2 * sizeof(uint8_t)> {
public:
    /**
     * Constructor for data user.
     * @param gyroId
     */
    GyroAuxilliaryDataset(object_id_t gyroId):
            StaticLocalDataSet(sid_t(gyroId, gyrodefs::GYRO_AUX_SET_ID)) {
        setAllVariablesReadOnly();
    }

    lp_var_t<uint8_t> gyroGeneralConfigReg42 = lp_var_t<uint8_t>(sid.objectId,
            gyrodefs::GENERAL_CONFIG_REG42, this);
    lp_var_t<uint8_t> gyroRangeConfigReg43 =  lp_var_t<uint8_t>(sid.objectId,
            gyrodefs::RANGE_CONFIG_REG43, this);
private:

    friend class GyroHandler;
    /**
     * Constructor for data creator.
     * @param hkOwner
     */
    GyroAuxilliaryDataset(HasLocalDataPoolIF* hkOwner):
            StaticLocalDataSet(hkOwner, gyrodefs::GYRO_AUX_SET_ID) {}
};


#endif /* MISSION_DEVICES_DEVICEPACKETS_GYROPACKETS_H_ */
