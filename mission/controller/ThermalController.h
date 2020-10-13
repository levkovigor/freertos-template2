#ifndef MISSION_CONTROLLER_THERMALCONTROLLER_H_
#define MISSION_CONTROLLER_THERMALCONTROLLER_H_

#include <fsfw/controller/ExtendedControllerBase.h>
#include "ctrldefinitions/ThermalCtrlPackets.h"


class ThermalController: public ExtendedControllerBase {
public:
	ThermalController(object_id_t objectId);
private:

	/** ExtendedControllerBase overrides */
    virtual ReturnValue_t handleCommandMessage(
            CommandMessage *message) override;

    virtual void performControlOperation() override;

    ReturnValue_t initializeAfterTaskCreation() override;

    ThermalCtrl::ThermalControllerTemperatureSet thermalControllerSet;
};


#endif /* MISSION_CONTROLLER_THERMALCONTROLLER_H_ */
