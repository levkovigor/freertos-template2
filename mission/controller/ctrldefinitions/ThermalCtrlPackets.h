#ifndef MISSION_CONTROLLER_CTRLDEFINITIONS_THERMALCTRLPACKETS_H_
#define MISSION_CONTROLLER_CTRLDEFINITIONS_THERMALCTRLPACKETS_H_

#include <mission/devices/devicedefinitions/ThermalSensorPacket.h>

namespace ThermalCtrl {

static constexpr uint32_t SAT_TEMPERATURES_SET_ID = 0;

enum ThermalCtrlPoolIds: lp_id_t {
    TEMP_TEST_HKB,
    TEMP_SYRLINKS,
    TEMP_MESHCAM,
    TEMP_CCSDS,
    TEMP_MGT,
    TEMP_PLOC
};

#include <fsfw/datapoollocal/StaticLocalDataSet.h>
#include <fsfw/datapoollocal/LocalPoolVariable.h>


/**
 * @brief   This will be the defintion of a dataset for all temperature
 *          measurements across the satellite.
 * @details
 * The thermal controller will have a complete set of all temperatures
 * across the satellite. It will read the data of the seperate sensors,
 * perform monitoring on them and then store the value in the own dataset.
 * The dataset can be used by ground to get the complete set of temperatures
 * from the satellite.
 */
class ThermalControllerTemperatureSet: public StaticLocalDataSet<35> {
public:
    ThermalControllerTemperatureSet(object_id_t ctrlId):
        StaticLocalDataSet(sid_t(ctrlId, SAT_TEMPERATURES_SET_ID)) {}


    lp_var_t<float> tempTestHkb = lp_var_t<float>(this->hkManager->getOwner(),
            ThermalCtrlPoolIds::TEMP_TEST_HKB, this);
    // todo: add all PT1000s temperatures here.
    lp_var_t<float> tempSyrlinks = lp_var_t<float>(this->hkManager->getOwner(),
            ThermalCtrlPoolIds::TEMP_SYRLINKS, this);
    lp_var_t<float> tempMeshcam = lp_var_t<float>(this->hkManager->getOwner(),
            ThermalCtrlPoolIds::TEMP_MESHCAM, this);
    lp_var_t<float> tempCcsds = lp_var_t<float>(this->hkManager->getOwner(),
            ThermalCtrlPoolIds::TEMP_CCSDS, this);
    lp_var_t<float> tempPloc = lp_var_t<float>(this->hkManager->getOwner(),
            ThermalCtrlPoolIds::TEMP_PLOC, this);

};

}

#endif /* MISSION_CONTROLLER_CTRLDEFINITIONS_THERMALCTRLPACKETS_H_ */
