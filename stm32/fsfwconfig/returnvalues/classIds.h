/*
 * classIds.h
 *
 *  Created on: 16.07.2018
 *      Author: mohr
 */

#ifndef CONFIG_RETURNVALUES_CLASSIDS_H_
#define CONFIG_RETURNVALUES_CLASSIDS_H_

#include <fsfw/returnvalues/FwClassIds.h>

/**
 * Source IDs starts at 73 for now
 * Framework IDs for ReturnValues run from 0 to 56
 * and are located inside <fsfw/returnvalues/FwClassIds.h>
 */
namespace CLASS_ID {
enum {
	MISSION_CLASS_ID_START = FW_CLASS_ID_COUNT,
	RS232_CHANNEL, //!< RS232
	I2C_CHANNEL, //!< I2C
	SPI_CHANNEL, //!< SPI
	GPS_HANDLER,  //!< GPS
	PUS_SERVICE_3, //!< HKS
    FILE_SYSTEM, //!< FLSY
    MGM_LIS3MDL, //!< MGML

};
}


#endif /* CONFIG_RETURNVALUES_CLASSIDS_H_ */
