#ifndef CONFIG_TMTC_SUBSYSTEMIDRANGES_H_
#define CONFIG_TMTC_SUBSYSTEMIDRANGES_H_

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
	PUS_SERVICE_9 = 89,
	PUS_SERVICE_23 = 91,
	DUMMY_DEVICE = 90,

	GPS_DEVICE = 105,

	SPI_COM_IF = 128,
	I2C_COM_IF = 138,

	TASK_MONITOR = 160,
	SYSTEM_STATE_TASK = 161
};
}

#endif /* CONFIG_TMTC_SUBSYSTEMIDRANGES_H_ */
