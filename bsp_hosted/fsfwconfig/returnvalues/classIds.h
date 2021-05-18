#ifndef FSFWCONFIG_RETURNVALUES_CLASSIDS_H_
#define FSFWCONFIG_RETURNVALUES_CLASSIDS_H_

#include "fsfw/returnvalues/FwClassIds.h"
#include "common/returnvalues/commonClassIds.h"

/**
 * @brief   CLASS_ID defintions which are required for custom returnvalues.
 */
namespace CLASS_ID {
enum: uint8_t {
	MISSION_CLASS_ID_START = COMMON_CLASS_ID_RANGE,
};
}


#endif /* FSFWCONFIG_RETURNVALUES_CLASSIDS_H_ */
