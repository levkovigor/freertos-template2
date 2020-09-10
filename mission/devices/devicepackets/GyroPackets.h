#ifndef MISSION_DEVICES_DEVICEPACKETS_GYROPACKETS_H_
#define MISSION_DEVICES_DEVICEPACKETS_GYROPACKETS_H_

#include <fsfw/datapoollocal/StaticLocalDataSet.h>
#include <fsfw/datapoollocal/LocalPoolVariable.h>

enum GyroPoolIds: lp_id_t {
    ANGULAR_VELOCITY_X,
	ANGULAR_VELOCITY_Y,
	ANGULAR_VELOCITY_Z,
	GENERAL_CONFIG_REG42,
	RANGE_CONFIG_REG43
};

class GyroPrimaryDataset: public StaticLocalDataSet<3 * sizeof(float)> {
public:
	GyroPrimaryDataset(sid_t sid):
		StaticLocalDataSet(sid),
		angVelocityX(GyroPoolIds::ANGULAR_VELOCITY_X,
				sid.objectId, pool_rwm_t::VAR_READ_WRITE, this),
		angVelocityY(GyroPoolIds::ANGULAR_VELOCITY_Y,
				sid.objectId, pool_rwm_t::VAR_READ_WRITE, this),
		angVelocityZ(GyroPoolIds::ANGULAR_VELOCITY_Z,
				sid.objectId, pool_rwm_t::VAR_READ_WRITE, this) {}

    lp_var_t<float> angVelocityX;
    lp_var_t<float> angVelocityY;
    lp_var_t<float> angVelocityZ;
};

class GyroAuxilliaryDataset: public StaticLocalDataSet<2 * sizeof(uint8_t)> {
public:
	GyroAuxilliaryDataset(sid_t sid):
		StaticLocalDataSet(sid),
		gyroGeneralConfigReg42(GyroPoolIds::GENERAL_CONFIG_REG42,
				sid.objectId, pool_rwm_t::VAR_READ_WRITE, this),
		gyroRangeConfigReg43(GyroPoolIds::RANGE_CONFIG_REG43,
				sid.objectId, pool_rwm_t::VAR_READ_WRITE, this) {}

	lp_var_t<uint8_t> gyroGeneralConfigReg42;
	lp_var_t<uint8_t> gyroRangeConfigReg43;
};

#endif /* MISSION_DEVICES_DEVICEPACKETS_GYROPACKETS_H_ */
