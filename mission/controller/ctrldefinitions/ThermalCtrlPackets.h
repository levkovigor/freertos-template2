#ifndef MISSION_CONTROLLER_CTRLDEFINITIONS_THERMALCTRLPACKETS_H_
#define MISSION_CONTROLLER_CTRLDEFINITIONS_THERMALCTRLPACKETS_H_

namespace ThermalController {

static constexpr uint32_t SAT_TEMPERATURES_SET_ID = 0;


#include <fsfw/datapoollocal/StaticLocalDataSet.h>
/**
 * @brief   This will be the defintion of a dataset for all temperature
 *          measurements across the satellite.
 */
class ThermalControllerTemperatureSet: public StaticLocalDataSet<35> {
public:
    ThermalControllerTemperatureSet(HasLocalDataPoolIF* hkOwner):
        StaticLocalDataSet(hkOwner, SAT_TEMPERATURES_SET_ID) {}
private:
    // todo: add all PT1000s here.
};

}

#endif /* MISSION_CONTROLLER_CTRLDEFINITIONS_THERMALCTRLPACKETS_H_ */
