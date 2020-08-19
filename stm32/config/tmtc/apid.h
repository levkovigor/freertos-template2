/*
 * apid.h
 *
 *  Application Process Definition: entity, uniquely identified by an application process ID (APID), capable of
 *	generating telemetry source packets and receiving telecommand packets
 *
 *	SOURCE APID: 0x73 / 115 / s
 *	APID is a 11 bit number
 *
 *  Created on: 06.12.2018
 *      Author: jakob
 */

#ifndef CONFIG_TMTC_APID_H_
#define CONFIG_TMTC_APID_H_

#include <stdint.h>

namespace apid{
	static const uint16_t SOURCE_OBSW = 0x73;
}


#endif /* CONFIG_TMTC_APID_H_ */
