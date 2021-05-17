#ifndef FSFWCONFIG_EVENTS_SUBSYSTEMIDRANGES_H_
#define FSFWCONFIG_EVENTS_SUBSYSTEMIDRANGES_H_

#include <cstdint>
#include <fsfw/events/fwSubsystemIdRanges.h>

/**
 * These IDs are part of the ID for an event thrown by a subsystem.
 * Numbers 0-80 are reserved for FSFW Subsystem IDs (framework/events/)
 */
namespace SUBSYSTEM_ID {
enum: uint8_t {
	SUBSYSTEM_ID_START = FW_SUBSYSTEM_ID_RANGE,
	PUS_SERVICE_2 = 82,
	PUS_SERVICE_3 = 83,
	PUS_SERVICE_5 = 85,
	PUS_SERVICE_6 = 86,
	PUS_SERVICE_8 = 88,
	PUS_SERVICE_23 = 103,

	DUMMY_DEVICE = 128,
	GPS_DEVICE = 129,
    MGM_LIS3MDL = 130,

    TEST_TASK = 132,

	SPI_COM_IF = 140,
	I2C_COM_IF = 141,
	UART_COM_IF = 142,

	TASK_MONITOR = 160,
	SYSTEM_STATE_TASK = 161,

	SD_CARD_HANDLER = 180,
	IMAGE_HANDLER = 181,

	CORE_CONTROLLER = 190

};
}

#endif /* FSFWCONFIG_EVENTS_SUBSYSTEMIDRANGES_H_ */
