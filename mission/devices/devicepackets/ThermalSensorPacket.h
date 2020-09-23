#ifndef MISSION_DEVICES_DEVICEPACKETS_THERMALSENSORPACKET_H_
#define MISSION_DEVICES_DEVICEPACKETS_THERMALSENSORPACKET_H_
#include <fsfw/datapoollocal/StaticLocalDataSet.h>
#include <fsfw/datapoollocal/LocalPoolVariable.h>

enum ThermalSensorPoolIds: lp_id_t {
    TEMPERATURE_C,
    FAULT_BYTE
};


class ThermalSensorDataset:
		public StaticLocalDataSet<sizeof(float) + sizeof(uint8_t)> {
		public:

	/**
	 * Constructor used by owner and data creators like device handlers.
	 * @param owner
	 * @param setId
	 */
	ThermalSensorDataset(HasLocalDataPoolIF* owner, uint32_t setId):
		StaticLocalDataSet(owner, setId) {
	}

	/**
	 * Constructor used by data users like controllers.
	 * @param sid
	 */
	ThermalSensorDataset(sid_t sid):
		StaticLocalDataSet(sid) {
	}

	lp_var_t<float> temperatureCelcius = lp_var_t<float>(
			ThermalSensorPoolIds::TEMPERATURE_C,
			sid.objectId, this);
	lp_var_t<uint8_t> errorByte = lp_var_t<uint8_t>(
			ThermalSensorPoolIds::FAULT_BYTE,
			sid.objectId, this);
};



#endif /* MISSION_DEVICES_DEVICEPACKETS_THERMALSENSORPACKET_H_ */

