/**
 * \file logicalAddresses.cpp
 *
 * \date 07.11.2019
 */

#ifndef CONFIG_DEVICES_LOGICALADDRESSES_H_
#define CONFIG_DEVICES_LOGICALADDRESSES_H_
#include <stdint.h>
#include <fsfw/devicehandlers/CookieIF.h>
#include <fsfwconfig/objects/systemObjectList.h>

namespace addresses {
	/* Logical addresses have uint32_t datatype */
	enum logicalAddresses: address_t {
		PCDU,

		/* Dummy and Test Addresses */
		DUMMY_ECHO = 129,
		DUMMY_GPS0  = 130,
		DUMMY_GPS1 = 131,
	};
}


#endif /* CONFIG_DEVICES_LOGICALADDRESSES_H_ */
