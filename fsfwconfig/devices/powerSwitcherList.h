/**
 * @file switcherList.h
 *
 * @date 28.11.2019
 */

#ifndef FSFWCONFIG_DEVICES_POWERSWITCHERLIST_H_
#define FSFWCONFIG_DEVICES_POWERSWITCHERLIST_H_

namespace switches {
	/* Switches are uint8_t datatype and go from 0 to 255 */
	enum switcherList {
		PCDU,
		GPS0,
		GPS1,
		PT1000,
		GYRO1,
		DUMMY = 129
	};

}


#endif /* FSFWCONFIG_DEVICES_POWERSWITCHERLIST_H_ */
