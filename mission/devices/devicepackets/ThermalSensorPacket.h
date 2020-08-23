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

    ThermalSensorDataset(sid_t sid):
            StaticLocalDataSet(sid),
            temperatureCelcius(ThermalSensorPoolIds::TEMPERATURE_C,
            		sid.objectId, pool_rwm_t::VAR_READ_WRITE, this),
            errorByte(ThermalSensorPoolIds::FAULT_BYTE,
            		sid.objectId, pool_rwm_t::VAR_READ_WRITE, this) {
    }

    lp_var_t<float> temperatureCelcius;
    lp_var_t<uint8_t> errorByte;
};



#endif /* MISSION_DEVICES_DEVICEPACKETS_THERMALSENSORPACKET_H_ */

