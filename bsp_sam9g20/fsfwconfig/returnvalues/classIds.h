#ifndef FSFWCONFIG_RETURNVALUES_CLASSIDS_H_
#define FSFWCONFIG_RETURNVALUES_CLASSIDS_H_

#include "fsfw/returnvalues/FwClassIds.h"
#include "common/returnvalues/commonClassIds.h"

/**
 * @brief   CLASS_ID defintions which are required for custom returnvalues.
 */
namespace CLASS_ID {
enum {
	MISSION_CLASS_ID_START = COMMON_CLASS_ID_RANGE,
	RS232_COM_IF, //RS232C
	RS232_POLLING, //RS232P
	RS485_POLLING,
	RS485_COM_IF, //RS485
	I2C_COM_IF, //I2C
	SPI_COM_IF, //SPIC
	PUS_PARSER, //PUSP
	SD_CARD_ACCESS, //SDCA
	SD_CARD_HANDLER, //SDCH
	SW_IMAGE_HANDLER, //SWIH
	SERIAL_ANALYZER, //SERA
};
}


#endif /* FSFWCONFIG_RETURNVALUES_CLASSIDS_H_ */
