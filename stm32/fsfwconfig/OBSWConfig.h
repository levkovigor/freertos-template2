#ifndef CONFIG_OBSWCONFIG_H_
#define CONFIG_OBSWCONFIG_H_

//#include "OBSWVersion.h"

#define OBSW_ADD_TEST_CODE 				1

//! Specify whether TMTC commanding via Ethernet is possible
#define OBSW_ETHERNET_TMTC_COMMANDING 	1
//! Only applies if TMTC commanding is enabled.
//! Specify whether LEDs are used to display Ethernet connection status.
#define OBSW_ETHERNET_USE_LEDS 			0



#ifdef __cplusplus
namespace config {
#endif

/* Add mission configuration flags here */
static const uint32_t MAX_STORED_TELECOMMANDS =         10;

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_OBSWCONFIG_H_ */
