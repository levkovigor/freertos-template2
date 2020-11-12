#ifndef FSFWCONFIG_TMTC_APID_H_
#define FSFWCONFIG_TMTC_APID_H_

#include <stdint.h>

/**
 * Application Process Definition: entity, uniquely identified by an
 * application process ID (APID), capable of generating telemetry source
 * packets and receiving telecommand packets
 *
 * SOURCE APID: 0x73 / 115 / s
 * APID is a 11 bit number
 */
namespace apid {
	static const uint16_t SOURCE_OBSW = 0x73;
}


#endif /* FSFWCONFIG_TMTC_APID_H_ */
