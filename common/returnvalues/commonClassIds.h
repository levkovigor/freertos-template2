#ifndef COMMON_RETURNVALUES_COMMONCLASSIDS_H_
#define COMMON_RETURNVALUES_COMMONCLASSIDS_H_

#include "fsfw/returnvalues/FwClassIds.h"

namespace CLASS_ID {
enum: uint8_t {
    COMMON_CLASS_ID_START = FW_CLASS_ID_COUNT,
    GPS_HANDLER, //GPSD
    MGM_LIS3MDL, //MGML
    COMMON_CLASS_ID_RANGE
};
}

#endif /* COMMON_RETURNVALUES_COMMONCLASSIDS_H_ */
