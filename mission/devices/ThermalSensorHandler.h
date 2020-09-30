#ifndef MISSION_DEVICES_THERMALSENSORHANDLER_H_
#define MISSION_DEVICES_THERMALSENSORHANDLER_H_

#include "devicedefinitions/ThermalSensorPacket.h"
#include <fsfw/devicehandlers/DeviceHandlerBase.h>
#include <fsfw/globalfunctions/PeriodicOperationDivider.h>

#include <array>
#include <cstdint>

/**
 * @brief       Device Handler for the thermal sensors
 * @details
 * Documentation of devices:
 * MAX31865 RTD converter:
 * https://datasheets.maximintegrated.com/en/ds/MAX31865.pdf
 * Pt1000 platinum resistance thermometers OMEGA F2020-1000-1/3B:
 * https://br.omega.com/omegaFiles/temperature/pdf/F1500_F2000_F4000.pdf
 *
 * The thermal sensor values are measured using the MAX31865 RTD converter IC
 * which creates digital values from the measured resistance of the Pt1000
 * devices which can be read with the SPI interface.
 * Refer to the SOURCE system schematic for the exact setup and number
 * of devices.
 *
 * @author      R. Mueller
 * @ingroup     devices
 */
class ThermalSensorHandler: public DeviceHandlerBase {
public:
	ThermalSensorHandler(object_id_t objectId, object_id_t comIF,
			CookieIF * comCookie, uint8_t switchId);
	virtual~ ThermalSensorHandler();

	// Configuration in 8 digit code:
	// 1. 1 for V_BIAS enabled, 0 for disabled
	// 2. 1 for Auto-conversion, 0 for off
	// 3. 1 for 1-shot enabled, 0 for disabled
	// 4. 1 for 3-wire disabled, 0 for disabled
	// 5./6. Fault detection:  00 for no action, 01 for automatic delay, 1
	// 	  0 for run fault detection with manual delay,
	// 	  11 for finish fault detection with manual delay
	// 7. Fault status:  1 for auto-clear, 0 for auto-clear off
	// 8. 1 for 50 Hz filter, 0 for 60 Hz filter (noise rejection filter)
	static constexpr uint8_t DEFAULT_CONFIG = 0b11000001;

	static constexpr float RTD_RREF_PT1000 = 4000.0; //!< Ohm
	static constexpr float RTD_RESISTANCE0_PT1000 = 1000.0; //!< Ohm
protected:
	void doStartUp() override;
	void doShutDown() override;
	ReturnValue_t buildNormalDeviceCommand(DeviceCommandId_t * id) override;
	ReturnValue_t buildTransitionDeviceCommand(DeviceCommandId_t * id) override;
	ReturnValue_t buildCommandFromCommand(DeviceCommandId_t deviceCommand,
			const uint8_t * commandData, size_t commandDataLen) override;
	void fillCommandAndReplyMap() override;
	ReturnValue_t scanForReply(const uint8_t *start, size_t remainingSize,
			DeviceCommandId_t *foundId, size_t *foundLen) override;
	ReturnValue_t interpretDeviceReply(DeviceCommandId_t id,
			const uint8_t *packet) override;
	void setNormalDatapoolEntriesInvalid() override;

	void debugInterface(uint8_t positionTracker = 0,
			object_id_t objectId = 0, uint32_t parameter = 0) override;
	uint32_t getTransitionDelayMs(Mode_t modeFrom, Mode_t modeTo) override;
	ReturnValue_t getSwitches(const uint8_t **switches,
			uint8_t *numberOfSwitches) override;

	void doTransition(Mode_t modeFrom, Submode_t subModeFrom) override;

	ReturnValue_t initializeLocalDataPool(LocalDataPool& localDataPoolMap,
	        LocalDataPoolManager& poolManager) override;
	ReturnValue_t initialize() override;
	//ReturnValue_t getDataSetHandle(sid_t sid) override;
private:
	const uint8_t switchId;

	enum class InternalState {
		NONE,
		WARMUP,
		CONFIGURE,
		REQUEST_CONFIG,
		RUNNING,
		REQUEST_FAULT_BYTE
	};

	InternalState internalState = InternalState::NONE;
	bool commandExecuted = false;

	dur_millis_t startTime = 0;
	uint8_t faultByte = 0;
	std::array<uint8_t, 3> commandBuffer { 0 };

	static constexpr DeviceCommandId_t CONFIG_CMD = 0x80;
	static constexpr DeviceCommandId_t REQUEST_CONFIG = 0x00;
	static constexpr DeviceCommandId_t REQUEST_RTD = 0x01;
	static constexpr DeviceCommandId_t REQUEST_FAULT_BYTE = 0x07;

	ThermalSensorDataset sensorDataset;
	sid_t sensorDatasetSid;

#ifdef DEBUG
	PeriodicOperationDivider debugDivider;
#endif
};

#endif /* MISSION_THERMALSENSORHANDLER_H_ */

