#ifndef MISSION_DEVICES_DEVICEPACKETS_THERMALSENSORPACKET_H_
#define MISSION_DEVICES_DEVICEPACKETS_THERMALSENSORPACKET_H_
#include <fsfw/datapoollocal/StaticLocalDataSet.h>
#include <fsfw/datapoollocal/LocalPoolVariable.h>
#include <systemObjectList.h>

namespace ThermalSensors {

enum ObjIds: object_id_t  {
    TEST_HKB_HANDLER = objects::SPI_Test_PT1000,
    SYRLINKS_HANDLER = objects::PT1000_Syrlinks_DEC1_O1,
    MGT_1_HANDLER = objects::PT1000_MGT1_DEC2,
    PLOC_HANDLER = objects::PT1000_PLOC_DEC4,
    MESHCAM_HANDLER = objects::PT1000_Camera_DEC1_O2
};

enum PoolIds: lp_id_t {
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
            PoolIds::TEMPERATURE_C,
            hkManager->getOwner(), this);
    lp_var_t<uint8_t> errorByte = lp_var_t<uint8_t>(
            PoolIds::FAULT_BYTE,
            hkManager->getOwner(), this);
};

}

#endif /* MISSION_DEVICES_DEVICEPACKETS_THERMALSENSORPACKET_H_ */

