/** 
 * @brief	Auto-generated object translation file.
 * @details
 * Contains 49 translations.
 * Generated on: 2021-05-18 16:31:20
 */
#include "translateObjects.h"

const char *ARDUINO_0_STRING = "ARDUINO_0";
const char *ARDUINO_1_STRING = "ARDUINO_1";
const char *ARDUINO_2_STRING = "ARDUINO_2";
const char *ARDUINO_3_STRING = "ARDUINO_3";
const char *ARDUINO_4_STRING = "ARDUINO_4";
const char *CORE_CONTROLLER_STRING = "CORE_CONTROLLER";
const char *SYSTEM_STATE_TASK_STRING = "SYSTEM_STATE_TASK";
const char *RS485_CONTROLLER_STRING = "RS485_CONTROLLER";
const char *DUMMY_GPS_COM_IF_STRING = "DUMMY_GPS_COM_IF";
const char *RS232_DEVICE_COM_IF_STRING = "RS232_DEVICE_COM_IF";
const char *I2C_DEVICE_COM_IF_STRING = "I2C_DEVICE_COM_IF";
const char *GPIO_DEVICE_COM_IF_STRING = "GPIO_DEVICE_COM_IF";
const char *SPI_POLLING_TASK_STRING = "SPI_POLLING_TASK";
const char *SPI_DEVICE_COM_IF_STRING = "SPI_DEVICE_COM_IF";
const char *DUMMY_ECHO_COM_IF_STRING = "DUMMY_ECHO_COM_IF";
const char *SD_CARD_HANDLER_STRING = "SD_CARD_HANDLER";
const char *FRAM_HANDLER_STRING = "FRAM_HANDLER";
const char *SOFTWARE_IMAGE_HANDLER_STRING = "SOFTWARE_IMAGE_HANDLER";
const char *UDP_TMTC_BRIDGE_STRING = "UDP_TMTC_BRIDGE";
const char *EMAC_POLLING_TASK_STRING = "EMAC_POLLING_TASK";
const char *SERIAL_TMTC_BRIDGE_STRING = "SERIAL_TMTC_BRIDGE";
const char *SERIAL_RING_BUFFER_STRING = "SERIAL_RING_BUFFER";
const char *SERIAL_POLLING_TASK_STRING = "SERIAL_POLLING_TASK";
const char *FSFW_OBJECTS_START_STRING = "FSFW_OBJECTS_START";
const char *PUS_SERVICE_1_VERIFICATION_STRING = "PUS_SERVICE_1_VERIFICATION";
const char *PUS_SERVICE_2_DEVICE_ACCESS_STRING = "PUS_SERVICE_2_DEVICE_ACCESS";
const char *PUS_SERVICE_3_HOUSEKEEPING_STRING = "PUS_SERVICE_3_HOUSEKEEPING";
const char *PUS_SERVICE_5_EVENT_REPORTING_STRING = "PUS_SERVICE_5_EVENT_REPORTING";
const char *PUS_SERVICE_8_FUNCTION_MGMT_STRING = "PUS_SERVICE_8_FUNCTION_MGMT";
const char *PUS_SERVICE_9_TIME_MGMT_STRING = "PUS_SERVICE_9_TIME_MGMT";
const char *PUS_SERVICE_17_TEST_STRING = "PUS_SERVICE_17_TEST";
const char *PUS_SERVICE_20_PARAMETERS_STRING = "PUS_SERVICE_20_PARAMETERS";
const char *PUS_SERVICE_200_MODE_MGMT_STRING = "PUS_SERVICE_200_MODE_MGMT";
const char *PUS_SERVICE_201_HEALTH_STRING = "PUS_SERVICE_201_HEALTH";
const char *HEALTH_TABLE_STRING = "HEALTH_TABLE";
const char *MODE_STORE_STRING = "MODE_STORE";
const char *EVENT_MANAGER_STRING = "EVENT_MANAGER";
const char *INTERNAL_ERROR_REPORTER_STRING = "INTERNAL_ERROR_REPORTER";
const char *TC_STORE_STRING = "TC_STORE";
const char *TM_STORE_STRING = "TM_STORE";
const char *IPC_STORE_STRING = "IPC_STORE";
const char *TIME_STAMPER_STRING = "TIME_STAMPER";
const char *FSFW_OBJECTS_END_STRING = "FSFW_OBJECTS_END";
const char *LED_TASK_STRING = "LED_TASK";
const char *AT91_I2C_TEST_TASK_STRING = "AT91_I2C_TEST_TASK";
const char *AT91_UART0_TEST_TASK_STRING = "AT91_UART0_TEST_TASK";
const char *AT91_UART2_TEST_TASK_STRING = "AT91_UART2_TEST_TASK";
const char *AT91_SPI_TEST_TASK_STRING = "AT91_SPI_TEST_TASK";
const char *NO_OBJECT_STRING = "NO_OBJECT";

const char* translateObject(object_id_t object) {
	switch( (object & 0xFFFFFFFF) ) {
	case 0x01010100:
		return ARDUINO_0_STRING;
	case 0x01010101:
		return ARDUINO_1_STRING;
	case 0x01010102:
		return ARDUINO_2_STRING;
	case 0x01010103:
		return ARDUINO_3_STRING;
	case 0x01010104:
		return ARDUINO_4_STRING;
	case 0x43001000:
		return CORE_CONTROLLER_STRING;
	case 0x43001005:
		return SYSTEM_STATE_TASK_STRING;
	case 0x43005000:
		return RS485_CONTROLLER_STRING;
	case 0x49001F00:
		return DUMMY_GPS_COM_IF_STRING;
	case 0x49005200:
		return RS232_DEVICE_COM_IF_STRING;
	case 0x49005300:
		return I2C_DEVICE_COM_IF_STRING;
	case 0x49005400:
		return GPIO_DEVICE_COM_IF_STRING;
	case 0x49005410:
		return SPI_POLLING_TASK_STRING;
	case 0x49005600:
		return SPI_DEVICE_COM_IF_STRING;
	case 0x4900AFFE:
		return DUMMY_ECHO_COM_IF_STRING;
	case 0x4D0073AD:
		return SD_CARD_HANDLER_STRING;
	case 0x4D008000:
		return FRAM_HANDLER_STRING;
	case 0x4D009000:
		return SOFTWARE_IMAGE_HANDLER_STRING;
	case 0x50000300:
		return UDP_TMTC_BRIDGE_STRING;
	case 0x50000400:
		return EMAC_POLLING_TASK_STRING;
	case 0x50000500:
		return SERIAL_TMTC_BRIDGE_STRING;
	case 0x50000550:
		return SERIAL_RING_BUFFER_STRING;
	case 0x50000600:
		return SERIAL_POLLING_TASK_STRING;
	case 0x53000000:
		return FSFW_OBJECTS_START_STRING;
	case 0x53000001:
		return PUS_SERVICE_1_VERIFICATION_STRING;
	case 0x53000002:
		return PUS_SERVICE_2_DEVICE_ACCESS_STRING;
	case 0x53000003:
		return PUS_SERVICE_3_HOUSEKEEPING_STRING;
	case 0x53000005:
		return PUS_SERVICE_5_EVENT_REPORTING_STRING;
	case 0x53000008:
		return PUS_SERVICE_8_FUNCTION_MGMT_STRING;
	case 0x53000009:
		return PUS_SERVICE_9_TIME_MGMT_STRING;
	case 0x53000017:
		return PUS_SERVICE_17_TEST_STRING;
	case 0x53000020:
		return PUS_SERVICE_20_PARAMETERS_STRING;
	case 0x53000200:
		return PUS_SERVICE_200_MODE_MGMT_STRING;
	case 0x53000201:
		return PUS_SERVICE_201_HEALTH_STRING;
	case 0x53010000:
		return HEALTH_TABLE_STRING;
	case 0x53010100:
		return MODE_STORE_STRING;
	case 0x53030000:
		return EVENT_MANAGER_STRING;
	case 0x53040000:
		return INTERNAL_ERROR_REPORTER_STRING;
	case 0x534f0100:
		return TC_STORE_STRING;
	case 0x534f0200:
		return TM_STORE_STRING;
	case 0x534f0300:
		return IPC_STORE_STRING;
	case 0x53500010:
		return TIME_STAMPER_STRING;
	case 0x53ffffff:
		return FSFW_OBJECTS_END_STRING;
	case 0x62000001:
		return LED_TASK_STRING;
	case 0x62007401:
		return AT91_I2C_TEST_TASK_STRING;
	case 0x62007402:
		return AT91_UART0_TEST_TASK_STRING;
	case 0x62007403:
		return AT91_UART2_TEST_TASK_STRING;
	case 0x62007404:
		return AT91_SPI_TEST_TASK_STRING;
	case 0xFFFFFFFF:
		return NO_OBJECT_STRING;
	default:
		return "UNKNOWN_OBJECT";
	}
	return 0;
}
