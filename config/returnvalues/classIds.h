#ifndef CONFIG_RETURNVALUES_CLASSIDS_H_
#define CONFIG_RETURNVALUES_CLASSIDS_H_

#include <fsfw/returnvalues/FwClassIds.h>

/**
 * @brief   CLASS_ID defintions which are required for custom returnvalues.
 */
namespace CLASS_ID {
enum {
	MISSION_CLASS_ID_START = FW_CLASS_ID_COUNT,
	RS232_COM_IF,
	RS232_POLLING,
	RS485_POLLING,
	RS485_COM_IF,
	I2C_COM_IF,
	SPI_COM_IF,
	GPS_HANDLER,
	PUS_SERVICE_3,
	PUS_PARSER,
	SD_CARD_ACCESS,
	SD_CARD_HANDLER,
};
}


#endif /* CONFIG_RETURNVALUES_CLASSIDS_H_ */
