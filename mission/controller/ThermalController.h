#ifndef MISSION_CONTROLLER_THERMALCONTROLLER_H_
#define MISSION_CONTROLLER_THERMALCONTROLLER_H_

#include <fsfw/controller/ExtendedControllerBase.h>
#include "ctrldefinitions/ThermalCtrlPackets.h"


class ThermalController: public ExtendedControllerBase {
public:
	ThermalController(object_id_t objectId);
private:

	// TODO: Add stubs for thermal components. Each device / assembly with one
	// or multiple redundant sensors will have a thermal component.

	/** ExtendedControllerBase overrides */
    virtual ReturnValue_t handleCommandMessage(
            CommandMessage *message) override;

    virtual void performControlOperation() override;

    virtual ReturnValue_t checkModeCommand(Mode_t mode, Submode_t submode,
            uint32_t *msToReachTheMode) override;

    ReturnValue_t initializeAfterTaskCreation() override;

    void handleChangedDataset(sid_t sid, store_address_t storeId) override;

    ThermalCtrl::ThermalControllerTemperatureSet thermalControllerSet;
};


#endif /* MISSION_CONTROLLER_THERMALCONTROLLER_H_ */
