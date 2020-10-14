#ifndef MISSION_DEVICES_DEVICEPACKETS_THERMALSENSORPACKET_H_
#define MISSION_DEVICES_DEVICEPACKETS_THERMALSENSORPACKET_H_

#include <fsfw/datapoollocal/StaticLocalDataSet.h>
#include <fsfw/datapoollocal/LocalPoolVariable.h>
#include <fsfw/devicehandlers/DeviceHandlerIF.h>
#include <systemObjectList.h>

namespace TSensorDefinitions {

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

static constexpr DeviceCommandId_t CONFIG_CMD = 0x80;
static constexpr DeviceCommandId_t REQUEST_CONFIG = 0x00;
static constexpr DeviceCommandId_t REQUEST_RTD = 0x01;
static constexpr DeviceCommandId_t REQUEST_FAULT_BYTE = 0x07;

static constexpr uint32_t THERMAL_SENSOR_SET_ID = REQUEST_RTD;

class ThermalSensorDataset:
        public StaticLocalDataSet<sizeof(float) + sizeof(uint8_t)> {
public:

    /**
     * Constructor used by owner and data creators like device handlers.
     * @param owner
     * @param setId
     */
    ThermalSensorDataset(HasLocalDataPoolIF* owner):
        StaticLocalDataSet(owner, THERMAL_SENSOR_SET_ID) {
    }

    /**
     * Constructor used by data users like controllers.
     * @param sid
     */
    ThermalSensorDataset(object_id_t objectId):
        StaticLocalDataSet(sid_t(objectId, THERMAL_SENSOR_SET_ID)) {
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

