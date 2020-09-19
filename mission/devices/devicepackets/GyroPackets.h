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
	GyroPrimaryDataset(HasLocalDataPoolIF* hkOwner, uint32_t setId):
		StaticLocalDataSet(hkOwner, setId) {}

	GyroPrimaryDataset(sid_t sid):
		StaticLocalDataSet(sid) {}

    lp_var_t<float> angVelocityX = lp_var_t<float>(
    		GyroPoolIds::ANGULAR_VELOCITY_X,
			sid.objectId, this);
    lp_var_t<float> angVelocityY = lp_var_t<float>(
    		GyroPoolIds::ANGULAR_VELOCITY_Y,
			sid.objectId, this);
    lp_var_t<float> angVelocityZ = lp_var_t<float>(
    		GyroPoolIds::ANGULAR_VELOCITY_Z,
			sid.objectId, this);
};

class GyroAuxilliaryDataset: public StaticLocalDataSet<2 * sizeof(uint8_t)> {
public:
	GyroAuxilliaryDataset(HasLocalDataPoolIF* hkOwner, uint32_t setId):
		StaticLocalDataSet(hkOwner, setId) {}

	GyroAuxilliaryDataset(sid_t sid):
		StaticLocalDataSet(sid) {}

	lp_var_t<uint8_t> gyroGeneralConfigReg42 = lp_var_t<uint8_t>(
			GyroPoolIds::GENERAL_CONFIG_REG42, sid.objectId, this);
	lp_var_t<uint8_t> gyroRangeConfigReg43 =  lp_var_t<uint8_t>(
			GyroPoolIds::RANGE_CONFIG_REG43,
			sid.objectId, this);
};

#endif /* MISSION_DEVICES_DEVICEPACKETS_GYROPACKETS_H_ */
